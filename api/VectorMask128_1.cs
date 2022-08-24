
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