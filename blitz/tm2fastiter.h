// -*- C++ -*-
/***************************************************************************
 * blitz/array/fastiter.h  Declaration of FastArrayIterator<P_numtype,N_rank>
 *
 * $Id$
 *
 * Copyright (C) 1997-2011 Todd Veldhuizen <tveldhui@acm.org>
 *
 * This file is a part of Blitz.
 *
 * Blitz is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Blitz is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with Blitz.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * Suggestions:          blitz-devel@lists.sourceforge.net
 * Bugs:                 blitz-support@lists.sourceforge.net    
 *
 * For more information, please see the Blitz++ Home Page:
 *    https://sourceforge.net/projects/blitz/
 *
 ****************************************************************************/
#ifndef BZ_ARRAY_TM2FASTITER_H
#define BZ_ARRAY_TM2FASTITER_H

#ifdef BZ_HAVE_STD
 #include <sstream>
#else
 #include <strstream.h>
#endif

#include <blitz/constpointerstack.h>
#include <blitz/bzdebug.h>
#include <blitz/shapecheck.h>
#include <blitz/prettyprint.h>

BZ_NAMESPACE(blitz)

// forward declaration
template<typename, int, int> class FastTM2Iterator;
template<typename, int, int> class FastTM2CopyIterator;


template<typename P_numtype, int N_rows, int N_columns, typename P_arraytype>
class FastTM2IteratorBase {
public:
  typedef P_numtype                T_numtype;
  typedef TinyMatrix<T_numtype, N_rows, N_columns> T_matrix;
  typedef FastTM2IteratorBase<P_numtype, N_rows, N_columns, P_arraytype> T_iterator;
    typedef const T_matrix& T_ctorArg1;
    typedef int            T_ctorArg2;    // dummy
  typedef FastTM2CopyIterator<P_numtype, N_rows, N_columns> T_range_result;

    static const int 
        numArrayOperands = 1, 
        numIndexPlaceholders = 0,
        rank_ = 2;

  FastTM2IteratorBase(const T_iterator& x)
        : data_(x.data_), array_(x.array_)
    { }

    void operator=(const T_iterator& x)
    {
      BZPRECONDITION(0);
      // doesn't this copy the data in x.array_ and then make data_
      // point to x's array? doesn't seem right
        array_ = x.array_;
        data_ = x.data_;
        stack_ = x.stack_;
        //stride_ = x.stride_;
    }

    FastTM2IteratorBase(const T_matrix& array)
        : array_(array)
    {
        data_   = array.data();
    }

    ~FastTM2IteratorBase()
    { }

  T_numtype operator()(TinyVector<int,2> i) const
    {
        return array_(i);
    }

  static int ascending(const int r) 
    {
      if (r<rank_)
	return T_matrix::isRankStoredAscending(r);
      else
	return INT_MIN;
    }

  static int ordering(const int r) 
    {
      if (r<rank_)
	return T_matrix::ordering(r);
      else
	return INT_MIN;
    }

  static int lbound(const int r) 
    { 
      if (r<rank_)
	return T_matrix::lbound(r);
      else
	return INT_MIN;
    }

  static int ubound(const int r)
    { 
      if (r<rank_)
	return T_matrix::ubound(r);
      else
	return INT_MAX;
    }
    
  //RectDomain<rank_> domain() const { return T_matrix::domain(); };

    T_numtype first_value() const { return *data_; }

    T_numtype operator*() const
    { return *data_; }

    T_numtype operator[](int i) const
    { return data_[i * stride_]; }

    T_numtype fastRead(sizeType i) const
  { return data_[i]; }

  static int suggestStride(int r) 
    { return T_matrix::stride(r); }
  
  static bool isStride(int r, diffType stride)
    { return T_matrix::stride(r) == stride; }

    void push(int position)
    {
        stack_[position] = data_;
    }
  
    void pop(int position)
    { 
        data_ = stack_[position];
    }

    void advance()
    {
      data_ += stride_;
    }

    void advance(int n)
    {
      data_ += n * stride_;
    }

    void loadStride(int rank)
    {
        stride_ = T_matrix::stride(rank);
    }

    const T_numtype * restrict data() const
    { return data_; }

  const T_matrix& array() const 
  {return array_; }

    void _bz_setData(const T_numtype* ptr)
    { data_ = ptr; }

    // this is needed for the stencil expression fastRead to work
    void _bz_offsetData(sizeType i)
    { data_ += i;}

    // and these are needed for stencil expression shift to work
    void _bz_offsetData(sizeType offset, int dim)
    { data_ += offset*T_matrix::stride(dim); }
  
    void _bz_offsetData(sizeType offset1, int dim1, sizeType offset2, int dim2)
    { data_ += offset1*T_matrix::stride(dim1); 
      data_ += offset2*T_matrix::stride(dim2); }

    int stride() const
    { return stride_; }

  static bool isUnitStride(int r) 
    { return T_matrix::stride(r) == 1; }

    void advanceUnitStride()
    { ++data_; }

  bool canCollapse(int outerLoopRank, int innerLoopRank) const
    { return T_matrix::canCollapse(outerLoopRank, innerLoopRank); }

    template<typename T_shape>
    bool shapeCheck(const T_shape& s) const
  { return areShapesConformable(s, T_matrix::length()); }

  /*
    // Experimental
    T_numtype& operator()(int i) const
    {
      return (T_numtype&)data_[i*T_matrix::stride(0)];
    }

    // Experimental
    T_numtype& operator()(int i, int j) const
    {
      return (T_numtype&)data_[i*T_matrix::stride(0) + j*T_matrix::stride(1)];
    }

    // Experimental

    void moveTo(int i)
    {
        data_ = &const_cast<T_matrix&>(array_)(i);
    }

    void moveTo(int i, int j)
    {
        data_ = &const_cast<T_matrix&>(array_)(i,j);
    }

    template<int N_rank2>
    void moveTo(const TinyVector<int,N_rank2>& i)
    {
        data_ = &const_cast<T_matrix&>(array_)(i);
    }

    // Experimental
    void operator=(T_numtype x)
    {   *const_cast<T_numtype*>(data_) = x; }

    // Experimental
    template<typename T_value>
    void operator=(T_value x)
    {   *const_cast<T_numtype*>(data_) = x; }

    // Experimental
    template<typename T_value>
    void operator+=(T_value x)
    { *const_cast<T_numtype*>(data_) += x; }

    // NEEDS_WORK: other operators
  
    // Experimental
    operator T_numtype() const
    { return *data_; }
  */

    // // Experimental
    // T_numtype shift(int offset, int dim) const
    // {
    //   return data_[offset*T_matrix::stride(dim)];
    // }

    // // Experimental
    // T_numtype shift(int offset1, int dim1, int offset2, int dim2) const
    // {
    //   return data_[offset1*T_matrix::stride(dim1) 
    // 		   + offset2*T_matrix::stride(dim2)];
    // }

    void prettyPrint(BZ_STD_SCOPE(string) &str, 
        prettyPrintFormat& format) const
    {
        if (format.tersePrintingSelected())
            str += format.nextArrayOperandSymbol();
        else if (format.dumpArrayShapesMode())
        {
#ifdef BZ_HAVE_STD
	    BZ_STD_SCOPE(ostringstream) ostr;
#else
            ostrstream ostr;
#endif
            ostr << T_matrix::shape();
            str += ostr.str();
        }
        else {
            str += "TinyMatrix<";
            str += BZ_DEBUG_TEMPLATE_AS_STRING_LITERAL(T_numtype);
            str += ",";

            char tmpBuf[10];
            sprintf(tmpBuf, "%d", N_rows);

            str += tmpBuf;
            str += ",";
            sprintf(tmpBuf, "%d", N_columns);

            str += tmpBuf;
            str += ">";
        }
    }

  // tiny matrices can't be sliced
  template<typename T1, typename T2 = nilArraySection, 
	   class T3 = nilArraySection, typename T4 = nilArraySection, 
	   class T5 = nilArraySection, typename T6 = nilArraySection, 
	   class T7 = nilArraySection, typename T8 = nilArraySection, 
	   class T9 = nilArraySection, typename T10 = nilArraySection, 
	   class T11 = nilArraySection>
  class SliceInfo {
  public:    
    typedef void T_slice;
};

protected:
  const T_numtype * restrict           data_;
  P_arraytype                          array_;
  ConstPointerStack<T_numtype,rank_>  stack_;
  diffType                             stride_;
};


template<typename P_numtype, int N_rows, int N_columns>
class FastTM2CopyIterator;


template<typename P_numtype, int N_rows, int N_columns>
class FastTM2Iterator : 
  public FastTM2IteratorBase<P_numtype, N_rows, N_columns,
			     const TinyMatrix<P_numtype, N_rows, N_columns>&> 
{
public:
  typedef FastTM2IteratorBase<P_numtype, N_rows, N_columns,
			      const TinyMatrix<P_numtype, N_rows, N_columns>&> T_base;
  typedef typename T_base::T_numtype T_numtype;
  typedef typename T_base::T_matrix T_matrix;
  typedef typename T_base::T_iterator T_iterator;
  typedef typename T_base::T_ctorArg1 T_ctorArg1;
  typedef typename T_base::T_ctorArg2 T_ctorArg2;
  typedef typename T_base::T_range_result T_range_result;
  
  using T_base::rank_;
  using T_base::numArrayOperands;
  using T_base::numIndexPlaceholders;

  // NB: this ctor does NOT preserve stack and stride
  // parameters.  This is for speed purposes.
  FastTM2Iterator(const FastTM2Iterator& x) 
    : T_base(x)
  { }

  FastTM2Iterator(const T_matrix& array) : T_base(array) {}
  
  using T_base::operator=;
  void operator=(const FastTM2Iterator<P_numtype, N_rows, N_columns>& x)
  {
    T_base::operator=(x);
  }
  
  using T_base::operator();

  // template<int N>
  // T_range_result operator()(const RectDomain<N>& d) const
  // {
  //   return T_range_result(T_base::array_(d));
  // }

  // template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
  // 	   typename T7, typename T8, typename T9, typename T10, typename T11>
  // FastTM2CopyIterator<T_numtype, T_base::template SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice::rank>
  // operator()(T1 r1, T2 r2, T3 r3, T4 r4, T5 r5, T6 r6, T7 r7, T8 r8, T9 r9, T10 r10, T11 r11) const
  // {
  //   typedef FastTM2CopyIterator<T_numtype, T_base::template SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice::rank> slice;

  //   return slice(array_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11));
  // }
  
};

/* This version of the FastTM2Iterator makes a COPY of the array
   it's pointing to. This makes it possible to return expressions of
   arrays that have gone out of scope, or to slice expressions. */
template<typename P_numtype, int N_rows, int N_columns>
class FastTM2CopyIterator : 
  public FastTM2IteratorBase<P_numtype, N_rows, N_columns, const TinyMatrix<P_numtype, N_rows, N_columns> >
{
public:
  typedef FastTM2IteratorBase<P_numtype, N_rows, N_columns, 
			      const TinyMatrix<P_numtype, N_rows, N_columns> > T_base;
  typedef typename T_base::T_numtype T_numtype;
  typedef typename T_base::T_matrix T_matrix;
  typedef typename T_base::T_iterator T_iterator;
  typedef typename T_base::T_ctorArg1 T_ctorArg1;
  typedef typename T_base::T_ctorArg2 T_ctorArg2;
  typedef typename T_base::T_range_result T_range_result;

  using T_base::rank_;
  using T_base::numArrayOperands;
  using T_base::numIndexPlaceholders;


  // NB: this ctor does NOT preserve stack and stride
  // parameters.  This is for speed purposes.
  FastTM2CopyIterator(const FastTM2CopyIterator& x) 
    : T_base(x)
  { }

  FastTM2CopyIterator(const T_matrix& array) : T_base(array) { }
  
  using T_base::operator=;
  void operator=(const FastTM2CopyIterator& x)
  {
    T_base::operator=(x);
  }
  
  using T_base::operator();

  // template<int N>
  // T_range_result operator()(const RectDomain<N>& d) const
  // {
  //   return T_range_result(T_base::array_(d));
  // }

  // template<typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
  // 	   typename T7, typename T8, typename T9, typename T10, typename T11>
  // FastTM2CopyIterator<T_numtype, T_base::template SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice::rank>
  // operator()(T1 r1, T2 r2, T3 r3, T4 r4, T5 r5, T6 r6, T7 r7, T8 r8, T9 r9, T10 r10, T11 r11) const
  // {
  //   typedef FastTM2CopyIterator<T_numtype, T_base::template SliceInfo<T1,T2,T3,T4,T5,T6,T7,T8,T9,T10,T11>::T_slice::rank> slice;

  //   return slice(array_(r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11));
  // }
};


BZ_NAMESPACE_END

#endif // BZ_ARRAY_FASTITER_H