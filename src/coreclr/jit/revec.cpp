// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX                                                                           XX
XX                               Lower                                       XX
XX                                                                           XX
XX  Preconditions:                                                           XX
XX                                                                           XX
XX  Postconditions (for the nodes currently handled):                        XX
XX    - All operands requiring a register are explicit in the graph          XX
XX                                                                           XX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

#include "jitpch.h"
#ifdef _MSC_VER
#pragma hdrstop
#endif

#include "revec.h"

#if !defined(TARGET_64BIT)
#include "decomposelongs.h"
#endif // !defined(TARGET_64BIT)


#ifdef FEATURE_HW_INTRINSICS
#ifdef TARGET_XARCH

#include "hwintrinsic.h"
#include "namedintrinsiclist.h"

VectorPack::VectorPack(Compiler* comp) : _pack1(nullptr), _pack2(nullptr), _revecd(nullptr), _comp(comp), _allocator(_comp->getAllocator(CMK_Generic))
{
    Init();
}

VectorPack::VectorPack(Compiler* comp, GenTree *pack1, GenTree *pack2) : _pack1(pack1), _pack2(pack2), _revecd(nullptr), _comp(comp), _allocator(_comp->getAllocator(CMK_Generic))
{
    Init();
}

void VectorPack::Init() 
{
    _nodes = new (_allocator) jitstd::vector<VectorPack*>{_allocator};
}

void VectorPack::AddEdge(VectorPack *to)
{
    for (auto p : *_nodes)
    {
        if (p == to)
            return;
    }
    _nodes->push_back(to);
}

#ifdef DEBUG
void VectorPack::DumpPack()
{
    JITDUMP("(");
    JITDUMP("(");
    Compiler::printTreeID(_pack1);
    JITDUMP(",")
    _comp->gtDispNodeName(_pack1);
    JITDUMP(")");
    JITDUMP(",");
    JITDUMP("(");
    Compiler::printTreeID(_pack2);
    JITDUMP(",")
    _comp->gtDispNodeName(_pack2);
    JITDUMP(")");
}
#endif

PackGraph::PackGraph(Compiler* comp) : _comp(comp), _allocator(_comp->getAllocator(CMK_Generic))
{
    _roots = new (_allocator) jitstd::vector<VectorPack*>{_allocator};
    _interner = new (_allocator) VectorPackInterner(_allocator);
}

VectorPack* PackGraph::MakePack(GenTree *oper1, GenTree *oper2, bool &isFreshPack)
{
    PackPair pair{oper1, oper2};
    VectorPack* pack; 
    if (_interner->Lookup(pair, &pack))
    {
        isFreshPack = false;
        return pack;
    }

    pack = new (_allocator) VectorPack{_comp, oper1, oper2};

    _interner->Set(pair, pack);
    isFreshPack = true;
    return pack;
}

void PackGraph::AddRootPack(VectorPack *pack)
{
    _roots->push_back(pack);
}

void PackGraph::AddEdge(VectorPack *v1, VectorPack *v2)
{
    v1->AddEdge(v2);
}

#ifdef DEBUG
void PackGraph::DumpGraph()
{
    JITDUMP("--------------------\n");
    JITDUMP("Pack Graph\n");
    JITDUMP("--------------------\n");

    PackSet ps(_comp->getAllocator());

    for (auto r : *_roots)
    {
        ps.Set(r, true);
        DumpGraphDfs(r, &ps);
    }

    JITDUMP("\n");
}

void PackGraph::DumpGraphDfs(VectorPack *p, PackSet *ps)
{
    JITDUMP("[");
    p->DumpPack();
    JITDUMP("] => [");

    for (auto e : *p->GetEdges())
    {
        e->DumpPack();
        JITDUMP(", ");
    }

    JITDUMP("]\n");

    if (p->GetRevecd() != nullptr)
    {
        JITDUMP("\n  ");
        _comp->gtDispLIRNode(p->GetRevecd());
    }

    for (auto e : *p->GetEdges())
    {
        if (ps->Lookup(e))
            continue;
        ps->Set(e, true);
        DumpGraphDfs(e, ps);
    }
}
#endif

bool Revectorizer::ExtractLclVarMemPairFromAddr(GenTree* addr, LclVarMemPair *sp)
{
    if (!addr->OperIs(GT_LCL_VAR) && !addr->OperIs(GT_LEA))
    {
        return false;
    }

    if (addr->OperIs(GT_LCL_VAR))
    {
        GenTreeLclVarCommon *lclVar = (GenTreeLclVarCommon*) addr;
        sp->base = comp->lvaGetDesc(lclVar->GetLclNum());
        sp->offset = 0;
    }
    else if (addr->OperIs(GT_LEA))
    {
        GenTreeAddrMode *addrMode = (GenTreeAddrMode*) addr;
        GenTree *base = addrMode->Base();

        if (!base->OperIs(GT_LCL_VAR))
            return false;

        GenTreeLclVarCommon *lclVar = (GenTreeLclVarCommon*)  base;


        sp->base = comp->lvaGetDesc(lclVar->GetLclNum());
        sp->offset = addrMode->Offset();
    }

    return true;
}

void Revectorizer::FindInitialPacks()
{

    CompAllocator allocator = comp->getAllocator(CMK_Generic);
    jitstd::vector<LclVarMemPair> stores{allocator};

    // Walk through LIR and look for stores
    for (GenTree* node : LIR::AsRange(_block))    
    {
        if (!node->OperIs(GT_HWINTRINSIC))
        {
            continue;
        }



        GenTreeHWIntrinsic* hwnode = (GenTreeHWIntrinsic*) node;
        GenTree* addr = nullptr;
        if (hwnode->OperIsMemoryStore(&addr))
        {
            LclVarMemPair sp;
            if (!ExtractLclVarMemPairFromAddr(addr, &sp))
                continue;

            sp.hwstore = hwnode;

            stores.push_back(sp);
        }
    }

    for (int i = 0; i < stores.size(); i++)
    {
        for (int j = 0; j < stores.size(); j++)
        {
            if (i == j) continue;
            if (stores[i].base != stores[j].base) continue;

            if (stores[i].offset < stores[j].offset && stores[j].offset - stores[i].offset == original_vector_width)
            {
                bool isFreshPack;
                VectorPack *pack = _graph->MakePack(stores[i].hwstore, stores[j].hwstore, isFreshPack);
                _graph->AddRootPack(pack);
            }
        }
    }
#ifdef DEBUG
    JITDUMP("--------------------\n");
    JITDUMP("Initial Vector Packs\n");
    JITDUMP("--------------------\n");

    jitstd::vector<VectorPack*> *roots = _graph->GetRootPacks();

    for (auto &p : *roots)
    {
        JITDUMP("P1=");
        comp->gtDispLIRNode(p->GetP1());
        JITDUMP("P2=");
        comp->gtDispLIRNode(p->GetP2());
        JITDUMP("\n");
    }
#endif
}


bool Revectorizer::IsIsomorphicMemoryLoad(GenTreeHWIntrinsic *o1, GenTreeHWIntrinsic *o2)
{
    assert(o1->OperIsMemoryLoad() && o2->OperIsMemoryLoad());

    GenTree* addr = nullptr;
    o1->OperIsMemoryLoad(&addr);

    LclVarMemPair o1pair;
    if (!ExtractLclVarMemPairFromAddr(addr, &o1pair))
    {
        return false;
    }

    o2->OperIsMemoryLoad(&addr);

    LclVarMemPair o2pair;
    if (!ExtractLclVarMemPairFromAddr(addr, &o2pair))
    {
        return false;
    }

    if (o1pair.base != o2pair.base) return false;

    if ((o1pair.offset < o2pair.offset && o1pair.offset + original_vector_width == o2pair.offset) ||
        (o2pair.offset < o1pair.offset && o2pair.offset + original_vector_width == o1pair.offset))
    {
        return true;
    }

    return false;
}

bool Revectorizer::IsIsomorphic(GenTreeHWIntrinsic *o1, GenTreeHWIntrinsic *o2)
{
    // Must be same operation
    if (o1->GetHWIntrinsicId() != o2->GetHWIntrinsicId()) return false;

    if (o1->OperIsMemoryLoad())
        return IsIsomorphicMemoryLoad(o1, o2);

    // Must return same type (because of intrinsics, previous check might supersede this)
    if (o1->GetSimdSize() == 0 || 
        o2->GetSimdSize() == 0 || 
        o1->GetSimdSize() != o2->GetSimdSize() ||
        o1->GetSimdBaseType() != o2->GetSimdBaseType())
    {
        return false;
    }

    // Operands must be off same type
    if (o1->GetOperandCount() != o2->GetOperandCount()) return false;

    for (size_t i = 1; i <= o1->GetOperandCount(); i++)
    {
        GenTree *o1op = o1->Op(i);
        GenTree* o2op = o2->Op(i);

        // Cheating, just checking the var_types, not a true check
        if (o1op->TypeGet() != o2op->TypeGet())
            return false;
    }

    return true;
}

bool Revectorizer::IsIsomorphicLclVar(GenTreeLclVarCommon *o1, GenTreeLclVarCommon *o2)
{
    // Just allow the pack to form for now
    return true;
}

bool Revectorizer::IsIsomorphicStoreLclVar(GenTreeLclVarCommon *o1, GenTreeLclVarCommon *o2)
{
    // Just allow the pack to form for now
    return true;
}

bool Revectorizer::IsIsomorphic(GenTree *o1, GenTree *o2)
{
    if (o1->OperKind() != o2->OperKind()) return false;

    switch (o1->OperGet())
    {
        case GT_HWINTRINSIC:
            return IsIsomorphic((GenTreeHWIntrinsic*) o1, (GenTreeHWIntrinsic*) o2);
        case GT_LCL_VAR:
            return IsIsomorphicLclVar((GenTreeLclVarCommon*) o1, (GenTreeLclVarCommon*) o2);
        case GT_STORE_LCL_VAR:
            return IsIsomorphicStoreLclVar((GenTreeLclVarCommon*) o1, (GenTreeLclVarCommon*) o2);
        default:
            return false;
    }
}

// Place holder O(n) to find a def for a GT_LCL_VAR until I figure out how to walk the actual use-def chain.
static GenTree* FindPrevDef(GenTreeLclVarCommon *node)
{
    for (GenTree *p = node->gtPrev; p != nullptr; p = p->gtPrev)
    {
        if (!p->OperIs(GT_STORE_LCL_VAR))
            continue;

        GenTreeLclVarCommon *lclVar = static_cast<GenTreeLclVarCommon*>(p);
        if (node->GetLclNum() == lclVar->GetLclNum())
            return lclVar;
    }

    return nullptr;
}

VectorPack* Revectorizer::TryFormPack(VectorPack* node, GenTree *o1, GenTree *o2, bool &isFreshPack)
{
    if (IsIsomorphic(o1, o2))
    {
        VectorPack* pack = _graph->MakePack(o1, o2, isFreshPack);
        _graph->AddEdge(node, pack);
        return pack;
    }

    return nullptr;
}

void Revectorizer::WalkPackGraph()
{
    jitstd::list<VectorPack*> worklist(comp->getAllocator(CMK_Generic));

    jitstd::vector<VectorPack*> *roots = _graph->GetRootPacks();

    for (auto p : *roots)
    {
        worklist.push_back(p);
    }

    // Really just does a single traversal to try to build packs
    while (!worklist.empty())
    {
        VectorPack* pack = worklist.front();
        worklist.pop_front();

        // For now, we just assume a point-wise check of the operands of the GenTree in the packs
        switch (pack->GetP1()->OperGet())
        {
            case GT_HWINTRINSIC:
            {
                GenTreeHWIntrinsic *o1 = static_cast<GenTreeHWIntrinsic*>(pack->GetP1());
                GenTreeHWIntrinsic *o2 = static_cast<GenTreeHWIntrinsic*>(pack->GetP2());

                if (o1->GetOperandCount() != o2->GetOperandCount())
                    break;

                for (int i = 1; i <= o1->GetOperandCount(); i++)
                {
                    bool isFreshPack = false;
                    VectorPack* possiblePack = TryFormPack(pack, o1->Op(i), o2->Op(i), isFreshPack);
                    if (possiblePack != nullptr && isFreshPack)
                        worklist.push_back(possiblePack);
                }

                break;
            }
            case GT_LCL_VAR:
            {
                GenTreeLclVarCommon *o1 = static_cast<GenTreeLclVarCommon*>(pack->GetP1());
                GenTreeLclVarCommon *o2 = static_cast<GenTreeLclVarCommon*>(pack->GetP2());

                GenTree *o1Def = FindPrevDef(o1);
                if (o1Def == nullptr)
                {
#ifdef DEBUG
                    JITDUMP("Could not find DEF for ");
                    Compiler::printTreeID(o1);
                    JITDUMP("\n");
                    break;
#endif
                }

                GenTree *o2Def = FindPrevDef(o2);
                if (o2Def == nullptr)
                {
#ifdef DEBUG
                    JITDUMP("Could not find DEF for ");
                    Compiler::printTreeID(o2);
                    JITDUMP("\n");
                    break;
#endif
                }
                
                bool isFreshPack = false;
                VectorPack* possiblePack = TryFormPack(pack, o1Def, o2Def, isFreshPack);
                if (possiblePack != nullptr && isFreshPack)
                    worklist.push_back(possiblePack);

                break;
            }

            case GT_STORE_LCL_VAR:
            {
                GenTreeLclVarCommon *o1 = static_cast<GenTreeLclVarCommon*>(pack->GetP1());
                GenTreeLclVarCommon *o2 = static_cast<GenTreeLclVarCommon*>(pack->GetP2());

                GenTree* o1Oper = o1->Data();
                GenTree* o2Oper = o2->Data();

                bool isFreshPack = false;
                VectorPack* possiblePack = TryFormPack(pack, o1Oper, o2Oper, isFreshPack);
                if (possiblePack != nullptr && isFreshPack)
                    worklist.push_back(possiblePack);

                    break;
            }


            default:
                break;

        }


    }

#ifdef DEBUG
    _graph->DumpGraph();
#endif

}

void Revectorizer::RevecPackGraph()
{
    PackSet ps(comp->getAllocator());

    for (auto e : *_graph->GetRootPacks())
    {
        ps.Set(e, true);
        DfsRevecPack(e, &ps);
    }

#ifdef DEBUG
    JITDUMP("After Revec\n");
    _graph->DumpGraph();
#endif

}

void Revectorizer::DfsRevecPack(VectorPack *p, PackSet *ps)
{
    for (auto e : *p->GetEdges())
    {
        if (ps->Lookup(e))
            continue;
        ps->Set(e, true);
        DfsRevecPack(e, ps);
    }

    RevecPack(p);
}

void Revectorizer::RevecPackHwLoad(VectorPack *p)
{
    GenTreeHWIntrinsic *o1 = static_cast<GenTreeHWIntrinsic*>(p->GetP1());
    GenTreeHWIntrinsic *o2 = static_cast<GenTreeHWIntrinsic*>(p->GetP2());

    // Form assumes the left side is the lesser of the offset
    GenTreeHWIntrinsic *revecd = 
        comp->gtNewSimdHWIntrinsicNode(
            TYP_SIMD32,
            o1->Op(1),
            NI_AVX_LoadVector256,
            o1->GetSimdBaseJitType(),
            o1->GetSimdSize() * 2, 
            o1->IsSimdAsHWIntrinsic()
        );

    p->SetRevecd(revecd);

    // Create a new local var
    unsigned revecdTemp = comp->lvaGrabTemp(true DEBUGARG("recvectorized temp"));
    comp->lvaTable[revecdTemp].lvType = TYP_SIMD32;
    p->SetRevecdLocalVar(revecdTemp);
}

void Revectorizer::RevecPackHwStore(VectorPack *p)
{
    GenTreeHWIntrinsic *o1 = static_cast<GenTreeHWIntrinsic*>(p->GetP1());
    GenTreeHWIntrinsic *o2 = static_cast<GenTreeHWIntrinsic*>(p->GetP2());

    GenTree *r1 = p->GetEdges()->at(0)->GetRevecd();

    // Form assumes the left side is the lesser of the offset
    GenTreeHWIntrinsic *revecd = 
        comp->gtNewSimdHWIntrinsicNode(
            o1->TypeGet(),
            o1->Op(1),
            r1,
            NI_AVX_Store,
            o1->GetSimdBaseJitType(),
            o1->GetSimdSize() * 2, 
            o1->IsSimdAsHWIntrinsic()
        );

    p->SetRevecd(revecd);
}

void Revectorizer::RevecPackHwOp(VectorPack *p)
{
    GenTreeHWIntrinsic *o1 = static_cast<GenTreeHWIntrinsic*>(p->GetP1());
    GenTreeHWIntrinsic *o2 = static_cast<GenTreeHWIntrinsic*>(p->GetP2());

    switch (o1->GetHWIntrinsicId())
    {
        case NI_SSE2_Add:
        {
            GenTree *r1 = p->GetEdges()->at(0)->GetRevecd();
            GenTree *r2 = p->GetEdges()->at(1)->GetRevecd();

            GenTree* revecd = 
                comp->gtNewSimdHWIntrinsicNode(
                    TYP_SIMD32,
                    r1,
                    r2,
                    NI_AVX2_Add,
                    o1->GetSimdBaseJitType(),
                    o1->GetSimdSize() * 2, 
                    o1->IsSimdAsHWIntrinsic()
                );

            p->SetRevecd(revecd);
        }
        default:
            JITDUMP("Cant handle yet\n");
            return;
    }

}

void Revectorizer::RevecPackLclVar(VectorPack *p)
{
    GenTreeLclVarCommon *o1 = static_cast<GenTreeLclVarCommon*>(p->GetP1());
    GenTreeLclVarCommon *o2 = static_cast<GenTreeLclVarCommon*>(p->GetP2());

    // Cheating, have to actually build up a candidate path and then check its legalization/cost
    // (Uses first for now)
    VectorPack *in = p->GetEdges()->front();
    // Presumes revecd comes from a store 
    GenTreeLclVarCommon *lclVarNode = static_cast<GenTreeLclVarCommon*>(in->GetRevecd());

    GenTreeLclVar *revecd = comp->gtNewLclvNode(lclVarNode->GetLclNum(), lclVarNode->TypeGet());
    p->SetRevecd(revecd);
}

void Revectorizer::RevecPackStoreLclVar(VectorPack *p)
{
    GenTreeLclVarCommon *o1 = static_cast<GenTreeLclVarCommon*>(p->GetP1());
    GenTreeLclVarCommon *o2 = static_cast<GenTreeLclVarCommon*>(p->GetP2());

    // Cheating, have to actually build up a candidate path and then check its legalization/cost
    // (Uses first for now)
    VectorPack *in = p->GetEdges()->front();
    GenTreeLclVar *revecd = comp->gtNewStoreLclVar(in->GetRevecdLocalVarNum(), in->GetRevecd());

    p->SetRevecd(revecd);
}

void Revectorizer::RevecPack(VectorPack *p)
{
    switch (p->GetP1()->OperGet())
    {
        case GT_HWINTRINSIC:
        {
            GenTreeHWIntrinsic *o1 = static_cast<GenTreeHWIntrinsic*>(p->GetP1());
            if (o1->OperIsMemoryLoad())
                RevecPackHwLoad(p);
            else if (o1->OperIsMemoryStore())
                RevecPackHwStore(p);
            else
                RevecPackHwOp(p);
            break;
        }
        case GT_LCL_VAR:
            RevecPackLclVar(p);
            break;
        case GT_STORE_LCL_VAR:
            RevecPackStoreLclVar(p);
            break;
    }
}

void Revectorizer::ReplaceNodes()
{
    PackSet ps(comp->getAllocator());
    LIR::Range &range = LIR::AsRange(_block);
    _removalNodes = new (comp->getAllocator()) jitstd::list<GenTree*>{comp->getAllocator()};

    for (auto e : *_graph->GetRootPacks())
    {
        ps.Set(e, true);
        DfsReplacePack(e, &ps, range);
    }

#ifdef DEBUG
    JITDUMP("After Revec\n");
    _graph->DumpGraph();
#endif

    // Do a hacky dead code elimination



}

void Revectorizer::DfsReplacePack(VectorPack *p, PackSet *ps, LIR::Range &range)
{
    // Currently, we replace the first pack and remove the second
    range.InsertAfter(p->GetP1(), p->GetRevecd());
    //_removalNodes->push_back(p->GetP2());
    //_removalNodes->push_back(p->GetP1());

    JITDUMP("Marking Operands [");
    Compiler::printTreeID(p->GetP2());
    JITDUMP(",");
    comp->gtDispNodeName(p->GetP2());
    JITDUMP("] Unused\n");

    range.Remove(p->GetP2(), true);


    range.Remove(p->GetP1());

    for (auto e : *p->GetEdges())
    {
        if (ps->Lookup(e))
            continue;
        ps->Set(e, true);
        DfsReplacePack(e, ps, range);
    }
}


/*
void Revectorizer::CleanDeadCode()
{
    LIR::Range &range = LIR::AsRange(_block);

    NodeSet removalSet(comp->getAllocator());

    while (!_removalNodes->empty())
    {
    }

    while (!_removalNodes->empty())
    {
        GenTree *n = _removalNodes->front();
        _removalNodes->pop_front();
        removalSet.Set(n, true, NodeSet::SetKind::Overwrite);

        JITDUMP("Checking node [")
        Compiler::printTreeID(n);
        JITDUMP(",")
        comp->gtDispNodeName(n);
        JITDUMP("] for removal\n");


        LIR::Use use;
        if (range.TryGetUse(n, &use))
        {
            GenTree *user = use.User();
            if (user)
            {
                JITDUMP("[")
                Compiler::printTreeID(n);
                JITDUMP(",")
                comp->gtDispNodeName(n);
                JITDUMP("] has use [")
                Compiler::printTreeID(user);
                JITDUMP(",")
                comp->gtDispNodeName(user);
                JITDUMP("]\n");

                _removalNodes->push_back(user);
            }
        }
    }

    for (auto it = removalSet.Begin(); !it.Equal(removalSet.End()); it++)
    {
        //range.Delete(comp, _block, it.Get());
        range.Remove(it.Get());
    }
}
*/

void Revectorizer::CleanDeadCode()
{
    LIR::Range &range = LIR::AsRange(_block);

    GenTree *n = _block->GetFirstLIRNode();

    while (n != nullptr)
    {
        if (n->IsUnusedValue())
        {
            JITDUMP("Cleaning unused value");
            JITDUMP("[")
            Compiler::printTreeID(n);
            JITDUMP(",")
            comp->gtDispNodeName(n);
            JITDUMP("]");


            GenTree *t = n->gtNext;
            range.Remove(n);
            n = t;
            continue;
        }
        n = n->gtNext;
    }

    n = _block->GetFirstLIRNode();
    while (n != nullptr)
    {
        LIR::Use use;
        if (n->OperIs(GT_LCL_VAR) && !range.TryGetUse(n, &use))
        {
            JITDUMP("Cleaning unused LCL VAR");
            JITDUMP("[")
            Compiler::printTreeID(n);
            JITDUMP(",")
            comp->gtDispNodeName(n);
            JITDUMP("]");

            GenTree *t = n->gtNext;
            range.Remove(n);
            n = t;
            continue;
        }
        n = n->gtNext;
    }
}

PhaseStatus Revectorizer::DoPhase()
{
    // Can only run on a single basic block for now, if the method has more, skip
    if (comp->fgBBcount != 1)
        return PhaseStatus::MODIFIED_NOTHING;

    _block = comp->fgFirstBB;

    FindInitialPacks();

    WalkPackGraph();

    RevecPackGraph();

    ReplaceNodes();

    CleanDeadCode();

    return PhaseStatus::MODIFIED_EVERYTHING;
}

#endif // TARGET_XARCH
#endif // FEATURE_HW_INTRINSICS 