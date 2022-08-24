public static class VectorMask
{
  public static bool IsHardwareAccelerated;  

  // The following we definitely want, standard boolean logic operators that are accelerated by AVX512

  public static VectorMask<T> Add<T>(VectorMask<T> left, VectorMask<T> right);
  public static VectorMask<T> And<T>(VectorMask<T> left, VectorMask<T> right);

  public static VectorMask<T> AndNot<T>(VectorMask<T> left, VectorMask<T> right);
  public static VectorMask<T> Not<T>(VectorMask<T> other);
  public static VectorMask<T> Or<T>(VectorMask<T> left, VectorMask<T> right);
  public static VectorMask<T> Xor<T>(VectorMask<T> left, VectorMask<T> right);

  public static VectorMask<T> Xnor<T>(VectorMask<T> left, VectorMask<T> right);

  // The following we want for AVX512 but have to consider how other arch fit

  public static VectorMask<T> ShiftLeft<T>(VectorMask<T> left, int count);
  public static VectorMask<T> ShiftRight<T>(VectorMask<T> left, int count);

  // Does this do element wise equality, or total equality check?

  public static VectorMask<T> Equals<T>(VectorMask<T> left, VectorMask<T> right);

  public static int LeadingZeroCount(VectorMask<T> mask);
  public static int TrailingZeroCount(VectorMask<T> mask);
  public static int PopCount(VectorMask<T> mask);

  public static bool GetElementCond(int index);
  public static bool SetElementCond(int index, bool cond);

  public static VectorMask<TTo> As<TFrom, TTo>(this VectorMask<TFrom> vector)
    where TFrom : struct
    where TTo : struct;

}