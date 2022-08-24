
public readonly struct Vector128<T> where T : struct
{
  public VectorMask128<T> AsMask();
}

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


public readonly struct VectorMask128<T> where T : struct 
{
  private readonly ushort _01;  // 16 bits is the most needed to mask 16 bytes (vector128 of byte)

  public static VectorMask128<T> Zero;
  public static VectorMask128<T> AllBitsSet;
  public static int Count;

  // The following we definitely want, standard boolean logic operators that are accelerated by AVX512

  public static unsafe VectorMask128<T> operator +(VectorMask128<T> left, VectorMask128<T> right);
  public static unsafe VectorMask128<T> operator &(VectorMask128<T> left, VectorMask128<T> right);
  public static unsafe VectorMask128<T> operator !(VectorMask128<T> other);
  public static unsafe VectorMask128<T> operator |(VectorMask128<T> left, VectorMask128<T> right);
  public static unsafe VectorMask128<T> operator ^(VectorMask128<T> left, VectorMask128<T> right);

  // The following we want for AVX512 but have to consider how other arch fit

  public static unsafe VectorMask128<T> operator <<(VectorMask128<T> other, int count);

  public static unsafe VectorMask128<T> operator >>(VectorMask128<T> other, int count);

  // Do we want these to be bit wise equality, or total equality, total equality is consistent with Vector ==.

  public static unsafe bool operator ==(VectorMask128<T> left, VectorMask128<T> right);

  public static unsafe bool operator !=(VectorMask128<T> left, VectorMask128<T> right);

  public static bool this[int index] => this.GetElementCond(index);

}

public readonly struct Vector<T> where T : struct
{
  public VectorMask<T> AsMask();
}

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

  // To be a bit consistent with Vector128 etc, this can return either all bits set or zero

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

public readonly struct VectorMask<T> where T : struct 
{
  // TODO: Do we force some kind of alignment like Vector does with a few fields?

  public static VectorMask<T> Zero;
  public static VectorMask<T> AllBitsSet;
  public static int Count;

  // The following we definitely want, standard boolean logic operators that are accelerated by AVX512

  public static unsafe VectorMask<T> operator +(VectorMask<T> left, VectorMask<T> right);
  public static unsafe VectorMask<T> operator &(VectorMask<T> left, VectorMask<T> right);
  public static unsafe VectorMask<T> operator !(VectorMask<T> other);
  public static unsafe VectorMask<T> operator |(VectorMask<T> left, VectorMask<T> right);
  public static unsafe VectorMask<T> operator ^(VectorMask<T> left, VectorMask<T> right);

  // The following we want for AVX512 but have to consider how other arch fit

  public static unsafe VectorMask<T> operator <<(VectorMask<T> other, int count);

  public static unsafe VectorMask<T> operator >>(VectorMask<T> other, int count);

  // Do we want these to be bit wise equality, or total equality, total equality is consistent with Vector ==.

  public static unsafe bool operator ==(VectorMask<T> left, VectorMask<T> right);

  public static unsafe bool operator !=(VectorMask<T> left, VectorMask<T> right);

  public static bool this[int index] => this.GetElementCond(index);
}