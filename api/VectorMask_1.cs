
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