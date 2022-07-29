// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

/*XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX                                                                           XX
XX                               Lower                                       XX
XX                                                                           XX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
*/

#ifndef _REVEC_H_
#define _REVEC_H_

#include "compiler.h"
#include "phase.h"
#include "jitstd.h"
#include "jithashtable.h"

#ifdef FEATURE_HW_INTRINSICS
#ifdef TARGET_XARCH

struct LclVarMemPair {
    GenTreeHWIntrinsic* hwstore;
    LclVarDsc *base;
    int offset;
};

struct PackPair : public JitKeyFuncsDefEquals<PackPair>
{
    GenTree* o1;
    GenTree* o2;

    PackPair() : o1(nullptr), o2(nullptr) { }

    PackPair(GenTree* p1, GenTree* p2) : o1(p1), o2(p2) { }

    friend bool operator==(const PackPair &x, const PackPair &y)
    {
        return (x.o1 == y.o1 && x.o2 == y.o2);
    }

    static unsigned GetHashCode(const PackPair& val)
    {
        return static_cast<unsigned>(reinterpret_cast<uintptr_t>(val.o1)) ^ static_cast<unsigned>(reinterpret_cast<uintptr_t>(val.o2));
    }

};

class VectorPack 
{
public:
    VectorPack(Compiler* );
    VectorPack(Compiler* , GenTree *, GenTree *);

    GenTree* GetP1() { return _pack1; };
    GenTree* GetP2() { return _pack2; };

    GenTree* GetRevecd() { return _revecd; };
    void SetRevecd(GenTree* revecd) { _revecd = revecd; }

    unsigned GetRevecdLocalVarNum() { return _revecdLocalVarNum; }
    void SetRevecdLocalVar(unsigned revecdLocalVarNum) { _revecdLocalVarNum = revecdLocalVarNum; } 

    bool IntroducesLocalVar() { return _revecdLocalVarNum != 0; }

    void AddEdge(VectorPack *);

    jitstd::vector<VectorPack*>* GetEdges()
    {
        return _nodes;
    }

#ifdef DEBUG
    void DumpPack();
#endif

private:
    void Init() ;

    GenTree* _pack1;
    GenTree* _pack2;

    GenTree* _revecd;
    unsigned _revecdLocalVarNum;

    jitstd::vector<VectorPack*>* _nodes;
    Compiler* _comp;
    CompAllocator _allocator;
};

typedef JitHashTable<VectorPack*, JitPtrKeyFuncs<VectorPack>, bool> PackSet;
typedef JitHashTable<GenTree*, JitPtrKeyFuncs<GenTree>, bool> NodeSet;

class PackGraph {
    typedef JitHashTable<PackPair, PackPair, VectorPack*> VectorPackInterner;

public:
    PackGraph(Compiler* );

    VectorPack* MakePack(GenTree *, GenTree *, bool &isFreshPack);
    void AddRootPack(VectorPack *);

    void AddEdge(VectorPack *, VectorPack *);

    jitstd::vector<VectorPack*>* GetRootPacks()
    {
        return _roots;
    }

#ifdef DEBUG
    void DumpGraph();
    void DumpGraphDfs(VectorPack *n, PackSet *ps);
#endif


private:
    jitstd::vector<VectorPack*> *_roots;
    Compiler* _comp;
    VectorPackInterner* _interner;
    CompAllocator _allocator;
};

class Revectorizer final : public Phase
{
public:
    inline Revectorizer(Compiler* comp) : Phase(comp, PHASE_REVECTORIZER)
    {
        _graph = new (comp->getAllocator(CMK_Generic)) PackGraph(comp);
    }
    virtual PhaseStatus DoPhase() override;

private:
    bool ExtractLclVarMemPairFromAddr(GenTree* , LclVarMemPair*);

    void FindInitialPacks();

    void WalkPackGraph();

    void RevecPackGraph();

    void ReplaceNodes();

    VectorPack* TryFormPack(VectorPack *, GenTree *, GenTree *, bool&);

    PackGraph* _graph;
    BasicBlock *_block;
    jitstd::list<GenTree*>* _removalNodes;

    const int original_vector_width = 16;
    const int target_vector_width = 32;


    bool IsIsomorphicMemoryLoad(GenTreeHWIntrinsic *, GenTreeHWIntrinsic *);
    bool IsIsomorphic(GenTreeHWIntrinsic *, GenTreeHWIntrinsic *);
    bool IsIsomorphicLclVar(GenTreeLclVarCommon *, GenTreeLclVarCommon *);
    bool IsIsomorphicStoreLclVar(GenTreeLclVarCommon *, GenTreeLclVarCommon *);
    bool IsIsomorphic(GenTree *, GenTree *);

    void DfsRevecPack(VectorPack *, PackSet *);
    void DfsReplacePack(VectorPack *, PackSet *, LIR::Range &range);

    void RevecPack(VectorPack *);

    void RevecPackHwLoad(VectorPack *);
    void RevecPackHwStore(VectorPack *);
    void RevecPackHwOp(VectorPack *);
    void RevecPackLclVar(VectorPack *);
    void RevecPackStoreLclVar(VectorPack *);

    void CleanDeadCode();
};

#endif // TARGET_XARCH
#endif // FEATURE_HW_INTRINSICS

#endif // _REVEC_H_
