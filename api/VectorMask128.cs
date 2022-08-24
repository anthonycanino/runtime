public static class VectorMask128
{
  public static bool IsHardwareAccelerated;  

// Create functions, have to align the type with the constant that can be used
  public static unsafe VectorMask128<T> Create(ushort mask);
  public static unsafe VectorMask128<byte> Create(ushort mask);
  public static unsafe VectorMask128<short> Create(ushort mask);
  public static unsafe VectorMask128<byte> Create(ushort mask);
  public static unsafe VectorMask128<byte> Create(ushort mask);


  // The following we definitely want, standard boolean logic operators that are accelerated by AVX512

  public static VectorMask128<T> Add<T>(VectorMask128<T> left, VectorMask128<T> right);
  public static VectorMask128<T> And<T>(VectorMask128<T> left, VectorMask128<T> right);

  public static VectorMask128<T> AndNot<T>(VectorMask128<T> left, VectorMask128<T> right);
  public static VectorMask128<T> Not<T>(VectorMask128<T> other);
  public static VectorMask128<T> Or<T>(VectorMask128<T> left, VectorMask128<T> right);
  public static VectorMask128<T> Xor<T>(VectorMask128<T> left, VectorMask128<T> right);
  public static VectorMask128<T> Xnor<T>(VectorMask128<T> left, VectorMask128<T> right);

  // The following we want for AVX512 but have to consider how other arch fit

  public static VectorMask128<T> ShiftLeft<T>(VectorMask128<T> left, int count);
  public static VectorMask128<T> ShiftRight<T>(VectorMask128<T> left, int count);

  // To be a bit consistent with Vector128 etc, this can return either all bits set or zero

  public static VectorMask128<T> Equals<T>(VectorMask128<T> left, VectorMask128<T> right);

  public static int LeadingZeroCount(VectorMask128<T> mask);
  public static int TrailingZeroCount(VectorMask128<T> mask);
  public static int PopCount(VectorMask128<T> mask);

  public static bool GetElementCond(int index);
  public static bool SetElementCond(int index, bool cond);

  public static VectorMask128<TTo> As<TFrom, TTo>(this VectorMask128<TFrom> vector)
    where TFrom : struct
    where TTo : struct;

}