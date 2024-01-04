# AVX10 Enablement and Optimizations in .NET Runtime

The following is a rough breakdown of high level tasks.

### 1. Make runtime and JIT aware of AVX10 ISA

Task: 

- Create `AVX10_1` and `AVX10_2` etc. ISA entries in `InstructionSetDesc.txt`. These will roughly correspond to the public API intrinsics for AVX10.

- Logic for new CPUID detection for AVX10, i.e., check vector length available, check AVX10 version, and read from the CPUID tree.

Opens:

1. JIT currently encodes an ISA (and available instructions) as a singular unit, i.e., if `AVX512VL` is available, an internal variable is set `X86_AVX512VL_ISA` to which all dependencies are check (can I use `AVX512VL` when compling `Vector`, can I use an `AVX512VL` instruction in emit etc.). `AVX10` decouples vector length from available instructions.

- We need to either introduce some feature that lets one query for available vector length, or we will have to have separate `AVX10_1_256_ISA`, `AVX10_1_512_ISA` to encode what is available for the compiler to target. 

2. Will we need to create a dependency on AVX512VL etc? I believe `AVX10` is meant to expose same instruction but exist in isolation from an ISA availability perspective.

Summary: No major blockers, would be straightforward but require more work than a new `AVX512` ISA due to the CPUID changes.

### 2. AVX10 intrinsic API proposal

Task:

- MSFT will want to expose a public API surface for working with AVX10. We can let them drive this, as they did for AVX512. But we will need to have a rough idea of the design as we will have to match the C# classes with the internal ISAs.

Open:

1. The decoupling of vector length from ISA, and its solution, will likely have to be reflected in the API as well. Something like this will probably be the answer

```C#
class Avx10 
{
  public bool IsSupported;

  class V256 
  {
    public bool IsSupported;
  }

  class V512 
  {
    public bool IsSupported;
  }
}
```

which means that we would represent `AVX10_V256` and `AVX10_V512` in the `InstructionSetDesc.txt` above.

Summary: No major blockers, but this will require a public API proposal and review process.

### 3. AVX10 intrinsic API codegen

Task:

- Each intrinsic will need to be mapped to the instruction it generates in the intrinsic tables. Given that most of `AVX10.2` overlaps with `AVX512VL`, this will likely be a copy paste effort and is very straightforward.

Open:

1. MSFT did the majority of this work last time, hopefully we can get them to contribute again.

Summary: No major blockers.

### 4. Allow additional 16 YMM registers for AVX10/Vector256

Task:

- Limit the set of new `YMM` registers to _only_ V256 if AVX10/256 is available, and the proper set for AVX10/512 (to be answered, see opens) in `lsra.cpp`, `lsrabuild.cpp`, and `lsraxarch.cpp`. The set of available registers (at runtime) is defined in the main compiler structure as `rbmAllFlt` and is set in `compiler.cpp`.

  - Right now, the JIT compiler will add `MM16` - `MM32` to the available set of float registers if `AVX512` is present. For `AVX10.2`, we cant simply add to this register set, as V128 would get assigned registers that are not available to be encoded.

  - This might hint that V256 should actually be transformed into AVX10_2 intrinsics, not AVX intrinsics. We would need a way to recognize that there exists an EVEX encoding for AVX10/256.

- Adjust `AVX512 ISA` codegen checks (`compOpportunisticallyDependsOn`) in relevant codepaths to allow for AVX10.1, AVX10.2 etc.

Open:

1. Will AVX10.2 on PCore have AVX512VL, or will it be limited to AVX10.2 512 and AVX10.1 256? 

Summary:

This will depend on the open question above, but regardless, will require a bit of work as the set of available registers on AVX10/256 is limited to 256 vector instructions only, which means we will have to somehow limit this.

### 5. Use new `AVX10` instruction forms of existing instructions for `Vector256` codegen

Task:

- Even though `vaddpd` will serve as the same vectorized add instruction for both `AVX2` and `AVX10/256`, there will exist two separate intrinsics nodes, `AVX2_Add_NI` and `AVX10_256_Add_NI`. Right now, as `Vector256` operations are imported in `hwintrinsicxarch.cpp`, they will get transformed into an `AVX` or `AVX2` intrinsic (or sequence of intrinsics). We will likely have to add additional code paths to transform the operations into `AVX10_2_256` intrinsics, particularly for register allocation purposes.

- We can likely import `Vector512` as `AVX512` on `AVX10/512` processors, as `AVX512` ISA has been frozen and it is covered by `AVX10/512`, _with a few notable exceptions_.

- Adjust `AVX512 ISA` codegen checks (`compOpportunisticallyDependsOn`) in relevant codepaths to allow for AVX10.1, AVX10.2 etc.

Open:

1. We need to make sure those _few notable exceptions_ stated in the AVX10 are not any of the instructions that are used by `Vector512` lowerings.

### 6. Utilize new `AVX10` operations instructions for .NET optimizations

Task:

- AVX10.2 introduces several new _classes_ of instructions, i.e., new acclerated operations not present in AVX512 (that apply to .NET as it stands):

1. AVX10 Compare Scalar FP with Enhanced Eflags Instructions

2. AVX10 Minmax Instructions

3. AVX10 Saturating Convert Instructions

  - Well known that these will be used as an optimization on the work Khushal is doing to implement saturating float/double conversions in .NET.

4. AVX10 Zero-Clearing FP Register Move Instructions

  - Move a double from one `xmm` register to another and clear upper 64 bits. `vmovsd` will clear upper 64 bits of `xmm` register when moving from a memory location. Might be cases where .NET has to conservatively clear upper bits of a `xmm` when copying or moving a double value.

Open:

1. Investigate use cases for (1) and (2) above.

### 7. Enable embedded rounding for `ymm` and adjust EVEX encoding.

Task:

- AVX10.2 allows for additional registers and embedded rounding for existing instructions (mostly overlaps with AVX512VL). For the embedded rounding on `ymm`, there are some EVEX encoding changes as stated in the AVX10 HAS doc (there is no embedded rounding for ymm in AVX512VL). Task is to make necessary adjustments in the emitter `emitxarch.cpp` and likely the `instrDesc` structure if needed.

Open:

- NONE

### 8. Enable existing `AVX512` optimizations to `Vector256`.

Task:

- Using additional `ymm` registers will likely force an `AVX10/256` form for some operations, i.e., `Vector256` comparisons. This will mean we have to use `kmask` registers, which had problems before Tanner introduced optimizations for them. Make sure the optimizations still/will apply. 

Open:

- Study the initial PR https://github.com/dotnet/runtime/pull/89059 and see how the optimizations are done. 


### General Questions 

1. In `AVX10/256`, `kmask` registers are 32 bit values, not 64. Does this lack of backward compatibility affect `Vector256`? I think the answer is no, but we have to double check.

