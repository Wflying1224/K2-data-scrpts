#ifndef __SPARSEMATRICES_H
#define __SPARSEMATRICES_H

#include <rectangularGrid.h>
#include <simplexGrid.h>
#include <rows.h>
#include <matrix.h>
#include <quoc.h>
#ifdef USE_EXTERNAL_GMM
#include <gmmIncludes.h>
#endif

namespace aol {

template <class T> class Matrix;

/** Basis class for sparse matrix classes that are organized row-wise
 *  Note that pointers to rows are stored and that NULL pointers mean identity rows. It is not clear whether this matrix always behaves as one would expect!
 */
template <typename DataType>
class GenSparseMatrix : public Matrix<DataType> {

  GenSparseMatrix() {}
  // Copying must be done in subclass where Row type is known
  explicit GenSparseMatrix ( const GenSparseMatrix& /*mat*/ ) {}
  // Assignment must be done in subclass where Row type is known
  GenSparseMatrix& operator= ( const GenSparseMatrix& /*mat*/ ) {}

public:
  typedef BitVector MaskType;

  GenSparseMatrix ( int Rows, int Columns )
      : Matrix<DataType> ( Rows, Columns ), rows ( Rows ),
      _diagEntry ( aol::ZOTrait<DataType>::one ) {
    for ( int i = 0; i < this->getNumRows(); ++i )
      rows[i] = NULL;
  }

  template <typename GridType>
  explicit GenSparseMatrix ( const GridType &Grid )
      : Matrix<DataType> ( Grid.getNumberOfNodes(), Grid.getNumberOfNodes() ),
      rows ( Grid.getNumberOfNodes() ),
      _diagEntry ( aol::ZOTrait<DataType>::one ) {
    init( );
  }

  //! resize Matrix, deleting old contents
  void reallocate ( const int Rows, const int Columns ) {
    int i;
    for ( i = Rows; i < this->_numRows; ++i ) {
      if ( rows[i] ) {
        delete rows[i];
        rows[i] = NULL;
      }
    }
    this->setZero();

    rows.resize ( Rows );
    for ( i = this->_numRows; i < Rows; ++i ) {
      rows[i] = NULL;
    }
    this->_numRows = Rows;
    this->_numCols = Columns;
  }

  virtual ~GenSparseMatrix() {}

  virtual Row<DataType>* newDefaultRow() const = 0;

  // *** other methods ***
  virtual DataType get ( int I, int J ) const {
#ifdef BOUNDS_CHECK
      if ( I < 0 || J < 0 || I >= static_cast<int> ( this->getNumRows() ) || J >= static_cast<int> ( this->getNumCols() ) ) {
        cerr << I << " " << J << " is out of bounds: " << this->getNumRows() << " " << this->getNumCols() << endl;
        throw aol::Exception ( "aol::GenSparseMatrix::get: Index out of bounds", __FILE__, __LINE__ );
      }
#endif
      if ( rows[I] ) {
        return ( rows[I]->get ( I, J ) );
      } else {
        if ( I == J ) {
          return _diagEntry;
        } else {
          return aol::ZOTrait<DataType>::zero;
        }
      }
    }

  virtual void set ( int I, int J, DataType Value ) {
#ifdef BOUNDS_CHECK
    if ( I < 0 || J < 0 || I >= static_cast<int> ( this->getNumRows() ) || J >= static_cast<int> ( this->getNumCols() ) ) {
      cerr << I << " " << J << " is out of bounds: " << this->getNumRows() << " " << this->getNumCols() << endl;
      throw aol::Exception ( "aol::GenSparseMatrix::set: Index out of bounds", __FILE__, __LINE__ );
    }
#endif
    if ( rows[I] )
      rows[I]->set ( I, J, Value );
    else if ( ( ( I == J ) && Value == _diagEntry ) || Value == aol::NumberTrait<DataType>::zero ) {
      // do nothing
#ifdef VERBOSE
      cerr << "aol::GenSparseMatrix<DataType>::set: setting non-existent entry (" << I << ", " << J << ") to "
      << Value << endl;
#endif
    } else
      throw Exception ( "Row does not exist.", __FILE__, __LINE__ );
  }

  virtual void add ( int I, int J, DataType Value ) {
#ifdef BOUNDS_CHECK
    if ( I < 0 || J < 0 || I >= static_cast<int> ( this->getNumRows() ) || J >= static_cast<int> ( this->getNumCols() ) ) {
      cerr << I << " " << J << " is out of bounds: " << this->getNumRows() << " " << this->getNumCols() << endl;
      throw aol::Exception ( "aol::GenSparseMatrix::add: Index out of bounds", __FILE__, __LINE__ );
    }
#endif
    if ( rows[I] ) {
      rows[I]->add ( I, J, Value );
    } else if ( Value == aol::NumberTrait<DataType>::zero ) {
      // do nothing
#ifdef VERBOSE
      cerr << "aol::GenSparseMatrix<DataType>::add: adding zero to diagonal entry" << endl;
#endif
    } else {
      cerr << rows[I] << " " << I << endl;
      throw Exception ( "Row does not exist.", __FILE__, __LINE__ );
    }
  }

  //! Adds vec1 \f$ \otimes \f$ vec2 to this.
  //! \author Toelkes
  inline virtual void addTensorProduct ( const Vector < DataType > &vec1, const Vector < DataType > &vec2 ) {
    addTensorProductMultiple ( vec1, vec2, 1.0 );
  }

  //! Adds factor * vec1 \f$ \otimes \f$ vec2 to this.
  //! \author Toelkes
  virtual void addTensorProductMultiple ( const Vector < DataType > &vec1, const Vector < DataType > &vec2, const DataType factor ) {
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      for ( int j = 0; j < this->getNumCols (); ++j ) {
        this->add ( i, j, factor * vec1[i] * vec2[j] );
      }
    }
  }

  //! Optimized version of sparse matrix-vector-multiplication.
  /*! warning default behaviour: if there is no row at given index, apply assumes _diagEntry on the diagonal. */
  void applyAdd ( const Vector<DataType> &Arg, Vector<DataType> &Dest ) const {
    if ( this->getNumRows() != Dest.size() || this->getNumCols() != Arg.size() ) {
      string msg = strprintf ( "aol::GenSparseMatrix::applyAdd: Cannot applyAdd %d by %d matrix from vector of size %d to vector of size %d.", this->getNumRows(), this->getNumCols(), Arg.size(), Dest.size() );
      throw ( Exception ( msg, __FILE__, __LINE__ ) );
    }

    for ( int i = 0; i <  this->getNumRows() ; ++i )
      if ( rows[i] )
        Dest[i] += rows[i]->mult ( Arg, i );
      else
        Dest [i] += _diagEntry * Arg[i];
  }

  //! Matrix-vector multiplication with masking functionality.
  //! Differently from apply and applyAdd, this function is not re-implemented
  //! in subclasses.
  void applyAddMasked ( const Vector<DataType> &Arg, Vector<DataType> &Dest,
                        const BitVector & Mask, IncludeWriteMode applyMode ) const {

    // check dimensions
    if ( this->getNumRows() != Dest.size() || this->getNumCols() != Arg.size() ) {
      char errmsg [ 1024 ];
      sprintf ( errmsg, "aol::GenSparseMatrix::apply: Cannot apply %d by %d matrix from vector of size %d to vector of size %d.", this->getNumRows(), this->getNumCols(), Arg.size(), Dest.size() );
      throw ( Exception ( errmsg, __FILE__, __LINE__ ) );
    }

    switch ( applyMode ) {
    case INCLUDE_ALL_WRITE_ALL:
      applyAddMasked<BitMaskFunctorTrue, &Row<DataType>::multMaskedFunctorTrue > ( Arg, Dest, Mask );
      break;
    case INCLUDE_BD_WRITE_INT:
      applyAddMasked<BitMaskFunctorIdentity, &Row<DataType>::multMaskedFunctorNegate > ( Arg, Dest, Mask );
      break;
    case INCLUDE_INT_WRITE_ALL:
      applyAddMasked<BitMaskFunctorTrue, &Row<DataType>::multMaskedFunctorIdentity > ( Arg, Dest, Mask );
      break;
    case INCLUDE_ALL_WRITE_INT:
      applyAddMasked<BitMaskFunctorIdentity, &Row<DataType>::multMaskedFunctorTrue > ( Arg, Dest, Mask );
      break;
    case INCLUDE_INT_WRITE_INT:
      applyAddMasked<BitMaskFunctorIdentity, &Row<DataType>::multMaskedFunctorIdentity > ( Arg, Dest, Mask );
      break;
    default:
      throw aol::Exception ( "aol::GenSparseMatrix::applyAddMasked: unknown IncludeWriteMode", __FILE__, __LINE__ );
    }
  }

  template <typename BitMaskFunctorType, DataType ( Row<DataType>::* multMasked ) ( const Vector<DataType> &, int Row, const BitVector & ) >
  void applyAddMasked ( const Vector<DataType> &Arg, Vector<DataType> &Dest,
                        const BitVector & Mask ) const {
    BitMaskFunctorType imFunctor;
    // now multiply only on desired nodes and write the result only to desired nodes
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for ( int i = 0; i <  this->getNumRows() ; ++i )
      if ( imFunctor ( Mask[i] ) ) {
        if ( rows[i] )
          Dest[i] += ( rows[i]->*multMasked ) ( Arg, i, Mask );
        else
          Dest [i] += _diagEntry * Arg[i];
      }
  }

  //! Optimized version of sparse matrix-vector-multiplication.
  /*! warning default behaviour: if there is no row at given index, apply assumes _diagEntry on the diagonal. */
  void apply ( const Vector<DataType> &Arg, Vector<DataType> &Dest ) const {
    if ( this->getNumRows() != Dest.size() || this->getNumCols() != Arg.size() ) {
      string msg = strprintf ( "aol::GenSparseMatrix::apply: Cannot apply %d by %d matrix from vector of size %d to vector of size %d.", this->getNumRows(), this->getNumCols(), Arg.size(), Dest.size() );
      throw ( Exception ( msg, __FILE__, __LINE__ ) );
    }
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for ( int i = 0; i <  this->getNumRows() ; ++i )
      if ( rows[i] )
        Dest[i] = rows[i]->mult ( Arg, i );
      else
        Dest [i] = _diagEntry * Arg[i];
  }


  //! Matrix-vector multiplication with masking functionality.
  //! Differently from apply and applyAdd, this function is not re-implemented
  //! in subclasses.
  //!
  //! Using a INCLUDE_*_WRITE_INT mode, actually a multiple of the identity is
  //! applied for bdry nodes, i. e. values are just copies from arg
  //! into "untouched" nodes.
  void applyMasked ( const Vector<DataType> &Arg, Vector<DataType> &Dest,
                     const BitVector & Mask, IncludeWriteMode applyMode ) const {

    // check dimensions
    if ( this->getNumRows() != Dest.size() || this->getNumCols() != Arg.size() ) {
      char errmsg [ 1024 ];
      sprintf ( errmsg, "aol::GenSparseMatrix::apply: Cannot apply %d by %d matrix from vector of size %d to vector of size %d.", this->getNumRows(), this->getNumCols(), Arg.size(), Dest.size() );
      throw ( Exception ( errmsg, __FILE__, __LINE__ ) );
    }

    switch ( applyMode ) {
    case INCLUDE_ALL_WRITE_ALL:
      applyMasked<BitMaskFunctorTrue, &Row<DataType>::multMaskedFunctorTrue > ( Arg, Dest, Mask );
      break;
    case INCLUDE_BD_WRITE_INT:
      applyMasked<BitMaskFunctorIdentity, &Row<DataType>::multMaskedFunctorNegate> ( Arg, Dest, Mask );
      break;
    case INCLUDE_ALL_WRITE_INT:
      applyMasked<BitMaskFunctorIdentity, &Row<DataType>::multMaskedFunctorTrue > ( Arg, Dest, Mask );
      break;
    case INCLUDE_INT_WRITE_ALL:
      applyMasked<BitMaskFunctorTrue, &Row<DataType>::multMaskedFunctorIdentity > ( Arg, Dest, Mask );
      break;
    case INCLUDE_INT_WRITE_INT:
      applyMasked<BitMaskFunctorIdentity, &Row<DataType>::multMaskedFunctorIdentity > ( Arg, Dest, Mask );
      break;
    default:
      throw aol::Exception ( "aol::GenSparseMatrix::applyMasked: unknown IncludeWriteMode", __FILE__, __LINE__ );
    }
  }

  template<typename BitMaskFunctorType, DataType ( Row<DataType>::* multMasked ) ( const Vector<DataType> &, int Row, const BitVector & ) >
  void applyMasked ( const Vector<DataType> &Arg, Vector<DataType> &Dest,
                     const BitVector & Mask ) const {
    BitMaskFunctorType imFunctor;
    // now multiply only on desired nodes and write the result only to desired nodes
#ifdef _OPENMP
#pragma omp parallel for
#endif
    for ( int i = 0; i <  this->getNumRows() ; ++i )
      if ( imFunctor ( Mask[i] ) ) {
        if ( rows[i] )
          Dest[i] = ( rows[i]->*multMasked ) ( Arg, i, Mask );
        else
          Dest [i] = _diagEntry * Arg[i];
      }

  }


  DataType multRow ( const Vector<DataType> &Arg, int RowNum ) const {
    if ( rows[ RowNum ] ) {
      return rows[ RowNum ]->mult ( Arg, RowNum );
    } else {
      return 0.;
    }
  }


  DataType rowSum ( const int I ) const {
    if ( rows[I] ) {
      return ( rows[I]->sum ( I ) );
#ifdef VERBOSE
    } else {
      cerr << "aol::GenSparseMatrix::rowSum: implicite identity row " << I << ", returning rowSum = 0 anyway." << endl;
#endif
    }
    return 0;
  }

  /** matrix is mapped to factor * matrix, this does not affect implicite diagonal (formerly identity) rows (identified by NULL pointer)
   */
  GenSparseMatrix<DataType>& operator*= ( const DataType factor ) {
    for ( int i = 0; i < this->_numRows; ++i ) {
      if ( rows[i] ) {
        rows[i]->scale ( i, factor );
#ifdef VERBOSE
      } else {
        cerr << "aol::GenSparseMatrix::scale: not scaling implicite identity row " << i << endl;
#endif
      }
    }

    return *this;
  }

  void newRow ( int I, Row<DataType> *NewRow ) {
    if ( rows[I] ) delete rows[I];
    rows[I] = NewRow;
  }

  //! clears all rows but keeps instances
  void setZero() {
    for ( int i = 0; i < this->_numRows; ++i ) {
      if ( rows[i] ) {
        rows[i]->setZero();
      }
    }
  }

  virtual void setRowToZero ( const int I ) {
    if ( rows[I] ) {
      rows[I]->setZero();
    }
  }


  //! deleteRow clears row and deletes instance, makes this row an implicit identity row
  void deleteRow ( int I ) {
    delete ( rows[I] );
    rows[I] = NULL;
  }

  //! scaleRow scales Ith row with factor unless it is implicit identity row
  void scaleRow ( const int RowNum, const DataType Factor ) {
    if ( rows[RowNum] ) {
      rows[RowNum]->scale ( RowNum, Factor );
#ifdef VERBOSE
    } else {
      cerr << "aol::GenSparseMatrix::scale: not scaling implicite identity row " << RowNum << endl;
#endif
    }
  }

  GenSparseMatrix<DataType>& addMultiple ( const GenSparseMatrix<DataType> &Matrix, const DataType factor ) {
    for ( int i = 0; i < this->_numRows; ++i ) {
      if ( !this->rows[ i ] && Matrix.rows[ i ] ) {
        this->rows[ i ] = newDefaultRow();
      }
      if ( this->rows[ i ] && Matrix.rows[ i] ) {
        this->rows[ i ]->addMultiple ( i, *Matrix.rows[ i ], factor );
      }
    }
    return ( *this );
  }

  using aol::Matrix<DataType>::operator+=;
  GenSparseMatrix<DataType>& operator+= ( const GenSparseMatrix<DataType> &Matrix ) {
    addMultiple ( Matrix, aol::NumberTrait<DataType>::one );
    return ( *this );
  }

  using aol::Matrix<DataType>::operator-=;
  GenSparseMatrix<DataType>& operator-= ( const GenSparseMatrix<DataType> &Matrix ) {
    addMultiple ( Matrix, -aol::NumberTrait<DataType>::one );
    return ( *this );
  }

  //! Return vector of row entries. Entries need not be sorted with respect to column index and zeros may be contained.
  void makeRowEntries ( vector<typename Row<DataType>::RowEntry > &vec, const int RowNum ) const {
    if ( rows[RowNum] ) {
      rows[RowNum]->makeRowEntries ( vec, RowNum );
    } else {
      vec.resize ( 1 );
      vec[0].col = RowNum;
      vec[0].value = _diagEntry;
    }
  }

  //! Same as makeRowEntries, only that the entries have to be sorted wrt column index.
  void makeSortedRowEntries ( vector<typename Row<DataType>::RowEntry > &vec, const int RowNum ) const {
    if ( rows[RowNum] ) {
      rows[RowNum]->makeSortedRowEntries ( vec, RowNum );
    } else {
      vec.resize ( 1 );
      vec[0].col = RowNum;
      vec[0].value = _diagEntry;
    }
  }

  void getRow ( int i, Vector<DataType> &v ) const {
    aol::Matrix<DataType>::getRow ( i, v );
  }

  const Row<DataType>& getRow ( int I ) const {
    return *rows[I];
  }


  bool checkForNANsAndINFs() const {
    for ( int i = 0; i < this->_numRows; ++i ) {
      if ( rows[i] && rows[i]->checkForNANsAndINFs() ) {
        return true;
      }
    }
    return false;
  }

  bool isSymmetric ( typename RealTrait<DataType>::RealType tol = 0 ) const {
    for ( int i = 0; i < this->_numRows; ++i )
      if ( rows[i] ) {
        vector< typename Row< DataType >::RowEntry > entries;
        makeRowEntries ( entries, i );
        for ( int j = 0; j < static_cast<int>(entries.size()); ++j )
          if ( Abs (entries[j].value - get ( entries[j].col, i ) ) > tol )
            return false;
      }
      return true;
   }

  /** Approximate comparison
   */
  bool isApproxEqual ( const GenSparseMatrix<DataType> &other, DataType epsilon ) {
    for ( int i = 0; i < this->_numRows; ++i ) {
      // check first if row only exists in one of them
      // (use XOR). Testing " != NULL " is
      // just to obtain a bool. One could equally cast
      // the pointers into booleans.
      if ( ( rows[i] != NULL ) ^ ( other.rows[i] != NULL ) )
        return false;

      // only test equality on rows if they exists
      if ( rows[ i ] && rows[ i ]->isApproxEqual ( i, *other.rows[ i ], epsilon ) == false )
        return false;
      else
        if ( _diagEntry != other._diagEntry )
          return false;
    }
    return true;
  }

  int numNonZeroes() const {
    int num = 0;
    for ( int i = 0; i < this->_numRows; ++i ) {
      if ( rows[i] )
        num += rows[i]->numNonZeroes();
      else
        if ( _diagEntry != aol::ZOTrait<DataType>::zero )
          num++;
    }
    return num;
  }

  int numStoredEntries() const {
    int num = 0;
    for ( int i = 0; i < this->_numRows; ++i ) {
      if ( rows[i] )
        num += rows[i]->numStoredEntries();
      else
        num++;
    }
    return num;
  }

  int numNonZeroRows() const {
    int num = 0;
    for ( int i = 0; i < this->_numRows; ++i )
      if ( rows[i] )
        num++;
      else
        if ( _diagEntry != aol::ZOTrait<DataType>::zero )
          num++;
    return num;
  }

  virtual int numNonZeroes ( int I ) const {
    if ( rows[I] )
      return rows[I]->numNonZeroes();
    else
      if ( _diagEntry != aol::ZOTrait<DataType>::zero )
        return 1;
    return 0;
  }

  virtual int numStoredEntries ( const int I ) const {
    if ( rows[I] )
      return rows[I]->numStoredEntries();
    else
      return 1;
  }

  //! reimplementation of transposition based on makeSortedRowEntries
  virtual void transposeTo ( aol::Matrix<DataType> &other_mat ) const {
    other_mat.setZero();
    for ( int i = 0; i < this->_numRows; ++i ) {
      vector<typename Row<DataType>::RowEntry > vec;
      makeSortedRowEntries ( vec, i );
      for ( typename vector<typename Row<DataType>::RowEntry >::iterator it = vec.begin(); it != vec.end(); ++it ) {
        other_mat.set ( it->col, i, it->value );
      }
    }
  }

  //! get diagonal entry
  DataType getDiag ( int i ) const {
    return ( get ( i, i ) );
  }

  DataType getUnsetRowsDiagEntry() const {
    return _diagEntry;
  }
  void setUnsetRowsDiagEntry ( DataType diagEntry ) {
    _diagEntry = diagEntry;
  }

protected:
  vector< Row<DataType>* > rows;

private:
  void init() {
    for ( int i = 0; i < this->getNumRows(); ++i )
      rows[i] = NULL;
  }
  DataType _diagEntry;
};


template < typename DataType > class SparseMatrixRowIterator;

/** a general, unstructured sparse matrix
 */
template <typename DataType>
class SparseMatrix : public GenSparseMatrix<DataType> {
  friend class SparseMatrixRowIterator < DataType >;

  bool _deleteRows;

public:
  SparseMatrix ( ) : GenSparseMatrix<DataType> ( 0, 0 ), _deleteRows ( true ) {}

  SparseMatrix ( int Rows, int Columns )
      : GenSparseMatrix<DataType> ( Rows, Columns ), _deleteRows ( true ) {
    for ( int i = 0; i < Rows; ++i )
      this->rows[i] = new SparseRow<DataType>;
  }

  template <typename GridType>
  explicit SparseMatrix ( const GridType &Grid )
      : GenSparseMatrix<DataType> ( Grid ), _deleteRows ( true ) {
    init();
  }

  template <qc::Dimension Dim>
  explicit SparseMatrix ( const qc::GridSize<Dim> &GridSize )
      : GenSparseMatrix<DataType> ( GridSize ), _deleteRows ( true ) {
    init();
  }

  //! Copying
  explicit SparseMatrix ( const SparseMatrix& mat, CopyFlag copyFlag = DEEP_COPY )
      : GenSparseMatrix<DataType> ( mat.getNumRows (), mat.getNumCols () ) {
    switch ( copyFlag ) {
    case STRUCT_COPY: {
      init();
      _deleteRows = true;
    }
    break;
    case DEEP_COPY: {
      this->rows.resize ( this->_numRows );
      for ( int i = 0; i < this->_numRows; ++i ) {
        const SparseRow<DataType>* oldrow = static_cast<const SparseRow<DataType>* > ( mat.rows [i] );
        this->rows [i] = new SparseRow<DataType> ( *oldrow );
      }
      _deleteRows = true;
    }
    break;
    case FLAT_COPY: {
      this->rows.resize ( this->_numRows );
      for ( int i = 0; i < this->_numRows; ++i ) {
        this->rows[i] = static_cast<SparseRow<DataType>* > ( mat.rows [i] );
      }
      _deleteRows = false;
    }
    break;
    default: {
      throw UnimplementedCodeException ( "This CopyFlag is not implemented yet.", __FILE__, __LINE__ );
    }
    break;
    }
  }

  Matrix<DataType>* clone ( CopyFlag copyFlag = DEEP_COPY ) const {
    SparseMatrix *clone = new SparseMatrix ( *this, copyFlag );
    return clone;
  }

  //! Assignment of another SparseMatrix of the same size.
  //! If necessary, this matrix has to be resized before assignment.
  SparseMatrix& operator= ( const SparseMatrix& mat ) {
    // Beware of self-assignment
    if ( this == &mat ) return *this;

    if ( mat.getNumRows() != this->getNumRows() || mat.getNumCols() != this->getNumCols() )
      throw Exception ( "SparseMatrix::operator= : dimensions don't match.", __FILE__, __LINE__ ); // and we do not want to change the size

    for ( int i = 0; i < this->_numRows; ++i ) {
      if ( !this->rows[i] && mat.rows[i] ) {
        const SparseRow<DataType>* oldrow = static_cast<const SparseRow<DataType>* > ( mat.rows [i] );
        this->rows [i] = new SparseRow<DataType> ( *oldrow );
      } else {
        * ( dynamic_cast< SparseRow<DataType>* > ( ( this->rows [i] ) ) ) = * ( dynamic_cast< SparseRow<DataType>* > ( ( mat.rows [i] ) ) );
      }
    }

    return *this;
  }

  // change size of matrix, destroying old contents.
  void reallocate ( const int new_rows, const int new_cols ) {
    GenSparseMatrix<DataType>::reallocate ( new_rows, new_cols );
    init();
  }

  //! resize matrix, keeping old contents (as far as possible)
  void resize ( const int new_rows, const int new_cols ) {
    if ( new_rows > this->_numRows ) {
      // increase number of rows

      this->rows.resize ( new_rows );
      for ( int i = this->_numRows; i < new_rows; ++i )
        this->rows[i] = new SparseRow<DataType>;

    } else if ( new_rows < this->_numRows ) {
      // decrease number of rows

      for ( int i = ( this->_numRows ) - 1 ; i >= new_rows; --i )
        delete ( this->rows[i] );
      this->rows.resize ( new_rows );

    } // else number of rows stays the same

    if ( new_cols > this->_numCols ) {

      // rows do not know how large they are so we needn't tell them

    } else if ( new_cols < this->_numCols ) {

      throw aol::UnimplementedCodeException ( "aol::SparseMatrix<DataType>::resize: Decreasing the number of columns not implemented yet.", __FILE__, __LINE__ );
      // rows do not know about their size, but we must not leave invalid entries in them.

    } // else number of columns stays the same.

    // if we got this far, we can set the new size.
    this->_numRows = new_rows;
    this->_numCols = new_cols;

  }

  //! \brief Resizes matrix without checking if old content can be kept.
  void destructiveResize ( const int new_rows, const int new_cols ) {
    if ( new_rows > this->_numRows ) {
      // increase number of rows
      this->rows.resize ( new_rows );
      for ( int i = this->_numRows; i < new_rows; ++i )
        this->rows[i] = new SparseRow<DataType>;
    }
    else if ( new_rows < this->_numRows ) {
      // decrease number of rows
      for ( int i = ( this->_numRows ) - 1 ; i >= new_rows; --i )
        delete ( this->rows[i] );
      this->rows.resize ( new_rows );

    } // else number of rows stays the same

    this->_numRows = new_rows;
    this->_numCols = new_cols;
  }

  //! \brief Destroy (remove) row i.
  void destroyRow ( const int i ) {
    this->rows.erase ( this->rows.begin () + i );
    --( this->_numRows );
  }

  //! \brief Insert a new row at position i.
  void insertRow ( const int i ) {
    this->rows.insert ( this->rows.begin () + i, new SparseRow<DataType>() );
    ++( this->_numRows );
  }

  virtual SparseRow<DataType>* newDefaultRow() const {
    return new SparseRow<DataType>();
  }

  virtual ~SparseMatrix() {
    destroy();
  }

  //! Delete entries that are exactly zero if these are stored for some reason
  void eraseZeroEntries () {
    for ( int i = 0; i < this->_numRows; ++i ) {
      dynamic_cast< aol::SparseRow<DataType>* >( this->rows[i] )->eraseZeroEntries();
    }
  }

  //! Adds multiple of one row to another
  void addMultipleRowToRow ( const int from, const int to, const DataType multiple ) {
    QUOC_ASSERT( from != to );

    // iterate over from-row and to-row simultaneously
    typename std::vector < typename aol::SparseRow<DataType>::qcCurMatrixEntry >::iterator it_from;
    typename std::vector < typename aol::SparseRow<DataType>::qcCurMatrixEntry >::iterator it_to;
    for ( it_from =  dynamic_cast< aol::SparseRow<DataType> * >(this->rows[from])->row.begin(),
          it_to =    dynamic_cast< aol::SparseRow<DataType> * >(this->rows[to])->row.begin();
          it_from != dynamic_cast< aol::SparseRow<DataType> * >(this->rows[from])->row.end(); ++it_from )  {
      // advance until column number in to-row is not smaller anymore or no entries are left
      while ( ( it_to != dynamic_cast< aol::SparseRow<DataType> * >(this->rows[to])->row.end() ) && ( it_to->col < it_from->col ) ) ++it_to;
      // no more entries in to-row or no match, insert before
      if ( ( it_to == dynamic_cast< aol::SparseRow<DataType> * >(this->rows[to])->row.end() ) || ( it_to->col != it_from-> col ) )
        // reassign iterator after insertion
        it_to = dynamic_cast< aol::SparseRow<DataType> * >(this->rows[to])->row.insert ( it_to, typename aol::SparseRow<DataType>::qcCurMatrixEntry ( it_from->col, multiple * it_from->value ) );
      // match
      else it_to->value += multiple * it_from->value;
    }
  }

  //! Adds multiple of one column to another
  void addMultipleColToCol ( const int from, const int to, const DataType multiple ) {
    QUOC_ASSERT( from != to );

    const int first  = aol::Min( from, to );
    const int second = aol::Max( from, to );

    // have to do it for all rows
    for ( int i = 0; i < this->getNumRows(); ++i )  {
      typename std::vector < typename aol::SparseRow<DataType>::qcCurMatrixEntry >::iterator it_first  = dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.begin();
      typename std::vector < typename aol::SparseRow<DataType>::qcCurMatrixEntry >::iterator it_second = dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.begin();
      // iterate until first entry is passed
      while ( ( it_first != dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.end() ) && ( it_first->col < first ) ) it_first++;
      // iterate further until second entry is passed
      it_second = it_first;
      while ( ( it_second != dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.end() ) && ( it_second->col < second ) ) it_second++;

      // it_first shall now correspond to from and it_second to to
      if ( to < from ) std::swap( it_first, it_second );

      // if from-entry was not found there is nothing to do
      if ( ( it_first != dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.end() ) && ( it_first->col == from ) )  {
        if ( ( it_second == dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.end() ) || ( it_second->col != to ) )
        // column number of it_second must be larger or end of row vector, insert before
          dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.insert ( it_second, typename aol::SparseRow<DataType>::qcCurMatrixEntry ( to, multiple * it_first->value ) );
        // match
        else it_second->value += multiple * it_first->value;
      }
    }
  }

  //! Set the whole row and col to zero except for the diagonal entry
  void setRowColToDiagonal ( const int index, const DataType diagEntry = 1. )  {
    // set row to zero
    this->rows[index]->setZero();
    // set col to zero
    for ( int i = 0; i < this->getNumRows(); ++i )  {
      typename std::vector < typename aol::SparseRow<DataType>::qcCurMatrixEntry >::iterator it;
      for ( it = dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.begin(); it != dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.end(); ++it )  {
        if ( it->col == index ) dynamic_cast< aol::SparseRow<DataType> * >(this->rows[i])->row.erase( it );
        if ( it->col >= index ) break;
      }
    }
    // set diagonal entry
    dynamic_cast< aol::SparseRow<DataType> * >(this->rows[index])->set( index,diagEntry );
  }

  //! Collapse a pair of rows and cols by adding multiples of one to the other and then setting the former row / col to zero except for the diagonal entry
  void collapseRowCol ( const int from, const int to, const DataType multiple, const DataType diagEntry = 1. )  {
    addMultipleRowToRow( from, to, multiple );
    addMultipleColToCol( from, to, multiple );
    setRowColToDiagonal( from, diagEntry );
  }

  //! Loads a matrix in the Harwell-Boeing format. Requires the external gmm.
  //! \author Berkels
  void loadHarwellBoeing ( const char *FileName ) {
#ifdef USE_EXTERNAL_GMM
    gmm::csc_matrix<DataType> gmmMat;
    gmm::Harwell_Boeing_load ( FileName, gmmMat );

    this->reallocate ( gmmMat.nrows(), gmmMat.ncols() );
    // These loops are not very efficient, so don't load matrices in performance critical code or
    // make this more efficient by only iterating of the non-zero entries of loaded matrix.
    for ( int i = 0; i < this->getNumRows(); ++i )
      for ( int j = 0; j < this->getNumCols(); ++j )
        if ( gmmMat ( i, j ) != 0 )
          this->set( i, j, gmmMat ( i, j ) );
#else
    aol::doNothingWithArgumentToPreventUnusedParameterWarning ( FileName );
    throw aol::Exception ( "Reading matrices in the Harwell-Boeing format requires the external gmm.", __FILE__, __LINE__ );
#endif // USE_EXTERNAL_GMM
  }

private:
  void init( ) {
    for ( int i = 0; i < this->getNumRows(); ++i )
      this->rows[i] = new SparseRow<DataType>;
  }

  void destroy() {
    if ( this->_deleteRows )
      for ( unsigned int i = 0; i < this->rows.size(); ++i ) {
        delete this->rows[ i ];
      }
  }
};


template <class RealType, class SparseOpType>
class RowEntryOp : public  aol::Op<aol::Vector<RealType> > {
  const SparseOpType &_sparseOp;
  const int _numRows;
public:
  RowEntryOp ( const SparseOpType &SparseOp, int NumRows ) : _sparseOp ( SparseOp ), _numRows ( NumRows ) {}

  int getNumRows() const {
    return _numRows;
  }

  void applyAdd ( const aol::Vector<RealType> &ArgVec, aol::Vector<RealType> &DestVec ) const {
    const RealType * Arg  = ArgVec.getData();
    RealType * Dest = DestVec.getData();
    vector<typename aol::Row<RealType>::RowEntry > rowEntries;
    for ( int i = 0; i < _numRows; ++i ) {
      _sparseOp.makeRowEntries ( rowEntries, i );
      for ( typename vector<typename aol::Row<RealType>::RowEntry >::iterator it = rowEntries.begin(); it != rowEntries.end(); ++it ) {
        Dest[ i ] += Arg[ it->col ] * it->value;
      }
    }
  }
};


/**
 * \brief A sparse matrix in triplet format.
 * \author Toelkes
 *
 * A sparse matrix class which stores its entries in a triplet format. If an entry appears more than once, the values are summed.
 * Useful for assembling, slow for arithmetic operations.
 */
template <typename DataType>
class TripletMatrix : public Matrix<DataType> {
protected:
  aol::Vector<int> _rowIndex;
  aol::Vector<int> _colIndex;
  aol::Vector<DataType> _value;

  template<typename> friend class TripletMatrixOffset;

  //! \brief Remove entries that have value zero.
  void removeZeroEntries () {
    for ( int i = 0; i < _value.size (); ++i ) {
      if ( _value[i] == 0.0 ) {
        _rowIndex.erase ( i );
        _colIndex.erase ( i );
        _value.erase ( i );
      }
    }
  }

  //! \brief Set a matrix entry to zero by setting _value for all its appearances to 0.0.
  inline void setEntryToZero ( int row, int col ) {
    for ( int k = 0; k < _rowIndex.size (); ++k ) {
      if ( _rowIndex[k] == row && _colIndex[k] == col ) {
        _value[k] = 0.0;
      }
    }
  }

  //! \brief Helper class for sorting indices by matrix position, first by row then by column.
  //! \author Toelkes
  class RowColIndexSorter {
  protected:
    const aol::Vector<int> &_rowIndex;
    const aol::Vector<int> &_colIndex;

  public:
    explicit RowColIndexSorter ( const aol::Vector<int> &rowIndex, const aol::Vector<int> &colIndex )
    : _rowIndex ( rowIndex ), _colIndex ( colIndex ) {}

    bool operator() ( int i, int j ) {
      return ( ( _rowIndex[i] < _rowIndex[j] ) || ( _rowIndex[i] == _rowIndex[j] && _colIndex[i] < _colIndex[j] ) );
    }
  };

  //! \brief Helper class for sorting indices by matrix position, first by column then by row.
  //! \author Toelkes
  class ColRowIndexSorter {
  protected:
    const aol::Vector<int> &_rowIndex;
    const aol::Vector<int> &_colIndex;

  public:
    explicit ColRowIndexSorter ( const aol::Vector<int> &rowIndex, const aol::Vector<int> &colIndex )
    : _rowIndex ( rowIndex ), _colIndex ( colIndex ) {}

    bool operator() ( int i, int j ) {
      return ( ( _colIndex[i] < _colIndex[j] ) || ( _colIndex[i] == _colIndex[j] && _rowIndex[i] < _rowIndex[j] ) );
    }
  };
public:
  //! \brief Standard constructor.
  explicit TripletMatrix ( const unsigned int numRows, const unsigned int numCols )
  : Matrix<DataType> ( numRows, numCols ) {}

  //! \brief Copy constructor.
  explicit TripletMatrix ( const TripletMatrix &mat, CopyFlag copyFlag = DEEP_COPY )
  : Matrix<DataType> ( mat.getNumRows (), mat.getNumCols () ) {
    switch ( copyFlag ) {
    case DEEP_COPY:
      _rowIndex = mat._rowIndex;
      _colIndex = mat._colIndex;
      _value = mat._value;
      break;
    case STRUCT_COPY:
      break;
    default:
      throw aol::UnimplementedCodeException ( "Copy flag not implemented", __FILE__, __LINE__ );
      break;
    }
  }

  //! \brief Standard destructor.
  virtual ~TripletMatrix () {}

  //! \brief Add value to entry at position (i, j).
  virtual void add ( int i, int j, DataType value ) {
#ifdef BOUNDS_CHECK
    if ( i > this->getNumRows () )
      throw aol::Exception ( "Row index is out of bounds", __FILE__, __LINE__, __FUNCTION__ );
    if ( j > this->getNumCols () )
      throw aol::Exception ( "Col index is out of bounds", __FILE__, __LINE__, __FUNCTION__ );
#endif

    _rowIndex.pushBack ( i );
    _colIndex.pushBack ( j );
    _value.pushBack ( value );
  }

  //! \brief Remove the i-th row and column.
  void removeRowCol ( unsigned int i ) {
    for ( int k = 0; k < _rowIndex.size (); ++k ) {
      if ( _rowIndex[k] == i || _colIndex[k] == i ) {
        _rowIndex.erase ( k );
        _colIndex.erase ( k );
        _value.erase ( k );
      }
      else {
        if ( _rowIndex[k] > i )
          --( _rowIndex[k] );
        if ( _colIndex[k] > i )
          --( _colIndex[k] );
      }
    }
  }

  //! \brief Set the i-th row to zero
  void setRowToZero ( int i ) {
    for ( int k = 0; k < _rowIndex.size (); ++k ) {
      if ( _rowIndex[k] == i ) {
        _value[k] = 0.0;
      }
    }
  }

  //! \brief Set the i-th column to zero
  void setColToZero ( int j ) {
    for ( int k = 0; k < _rowIndex.size (); ++k ) {
      if ( _colIndex[k] == j ) {
        _value[k] = 0.0;
      }
    }
  }

  //! \brief Apply method for compatibility with other matrices.
  //! \warning Not implemented.
  virtual void apply ( const aol::Vector<DataType> &, aol::Vector<DataType> & ) const {
    throw aol::UnimplementedCodeException ( "Apply not implemented...", __FILE__, __LINE__ );
  }

  //! \brief applyAdd method for compatibility with other matrices.
  //! \warning Not implemented.
  virtual void applyAdd ( const aol::Vector<DataType> &, aol::Vector<DataType> & ) const {
    throw aol::UnimplementedCodeException ( "ApplyAdd not implemented...", __FILE__, __LINE__ );
  }

  //! \brief Set the matrix to zero.
  virtual void setZero () {
    _rowIndex.reallocateClear ( 0 );
    _colIndex.reallocateClear ( 0 );
    _value.reallocateClear ( 0 );
  }

  void reallocate ( const int Rows, const int Columns ) {
    setZero();
    this->_numRows = Rows;
    this->_numCols = Columns;
  }

  //! \brief Get matrix entry (row, col).
  virtual DataType get ( int row, int col ) const {
    DataType ret = static_cast<DataType> ( 0 );

    for ( int i = 0; i < _rowIndex.size (); ++i ) {
      if ( _rowIndex[i] == row && _colIndex[i] == col )
        ret += _value[i];
    }

    return ret;
  }

  //! \brief Set matrix entry (row, col) to value.
  virtual void set ( int row, int col, DataType value ) {
    setEntryToZero ( row, col );
    add ( row, col, value );
  }

  //! \brief Erase value at (i, j).
  void eraseValue ( int row, int col ) {
    int k = 0;
    while ( k < _rowIndex.size () ) {
      if ( _rowIndex[k] == row && _colIndex[k] == col ) {
        _rowIndex.erase ( k );
        _colIndex.erase ( k );
        _value.erase ( k );
      }
      // Increment only, if the kth element has not been removed.
      // Otherwise, the former k+1st element is now the kth element.
      else {
        ++k;
      }
    }
  }

  //! \brief Find duplicate entries and sum these entries into one entry.
  void sumDuplicates () {
    std::vector<unsigned int> index ( _rowIndex.size () );
    for ( unsigned int i = 0; i < index.size (); ++i )
      index[i] = i;

    RowColIndexSorter rcSorter ( _rowIndex, _colIndex );
    std::sort ( index.begin (), index.end (), rcSorter );

    unsigned int i = 0;
    unsigned int j;
    while ( i < index.size () - 1 ) {
      j = i + 1;
      while ( _rowIndex[index[j]] == _rowIndex[index[i]] && _colIndex[index[j]] == _colIndex[index[i]] ) {
        _value[index[i]] += _value[index[j]];
        _value[index[j]] = static_cast<DataType> ( 0 );

        // If the last index has been processed, break.
        if ( ++j >= index.size () )
          break;
      }
      i += ( j - i );
    }

    removeZeroEntries ();
  }

  void getRowColIndexSorting ( std::vector<unsigned int> &index ) const {
    getIndexSorting<RowColIndexSorter> ( index );
  }

  void getColRowIndexSorting ( std::vector<unsigned int> &index ) const {
    getIndexSorting<ColRowIndexSorter> ( index );
  }

  template<typename SorterType>
  void getIndexSorting ( std::vector<unsigned int> &index ) const {
    index.resize ( _rowIndex.size () );
    for ( unsigned int i = 0; i < index.size (); ++i )
      index[i] = i;

    SorterType sorter ( _rowIndex, _colIndex );
    std::sort ( index.begin (), index.end (), sorter );
  }

  //! \brief Get reference to row index vector.
  const aol::Vector<int>& getRowIndexReference () const {
    return _rowIndex;
  }

  //! \brief Get reference to column index vector.
  const aol::Vector<int>& getColIndexReference () const {
    return _colIndex;
  }

  //! \brief Get reference to row value vector.
  const aol::Vector<DataType>& getValueReference () const {
    return _value;
  }

  //! \brief Converts the triplet matrix into a SparseMatrix.
  //! \param[out] sparseMatrix Will have the same entries as this, after the conversion (existing entries are truncated).
  void toSparseMatrix ( aol::SparseMatrix<DataType> &sparseMatrix ) {
    sparseMatrix.resize ( this->getNumRows (), this->getNumCols () );
    sparseMatrix.setZero ();

    for ( int i = 0; i < _value.size (); ++i ) {
      sparseMatrix.add ( _rowIndex[i], _colIndex[i], _value[i] );
    }
  }

  //! Adds multiple of one row to another
  void addMultipleRowToRow ( const int from, const int to, const DataType multiple ) {
    QUOC_ASSERT( from != to );
    for ( int k = 0; k < _rowIndex.size (); ++k ) {
      if ( _rowIndex[k] == from ) {
        add( to, _colIndex[k], multiple * _value[k] );
      }
    }
  }

  //! Adds multiple of one column to another
  void addMultipleColToCol ( const int from, const int to, const DataType multiple ) {
    QUOC_ASSERT( from != to );
    for ( int k = 0; k < _rowIndex.size (); ++k ) {
      if ( _colIndex[k] == from ) {
        add( _rowIndex[k], to, multiple * _value[k] );
      }
    }
  }

  //! Set the whole row and col to zero except for the diagonal entry
  void setRowColToDiagonal ( const int index, const DataType diagEntry = 1. )  {
    setRowToZero( index );
    setColToZero( index );
    add( index, index, diagEntry );
  }
};

/**
 * \brief Wrapper for using the aol::TripletMatrix in a block matrix.
 * \author Toelkes
 *
 * This class takes a TripletMatrix, sizes and offsets and maps matrix accesses
 * to the sub block given by these values.
 */
template <typename DataType>
class TripletMatrixOffset : public Matrix<DataType> {
protected:
  TripletMatrix<DataType> &_mat;
  int _rowOffset;
  int _colOffset;

public:
  //! \brief Standard constructor. Create matrix of size (0, 0).
  explicit TripletMatrixOffset ()
  : Matrix<DataType> ( 0, 0 ), _mat ( TripletMatrix<DataType> ( 0, 0 ) ), _rowOffset ( 0 ), _colOffset ( 0 ) {}

  //! \brief Constructor for compatibility with usual matrices.
  //!
  //! Does not make sense but is e.g. used in block matrices
  TripletMatrixOffset ( const int numRows, const int numCols )
  : Matrix<DataType> ( numRows, numCols ), _mat ( *(new TripletMatrix<DataType> ( numRows, numCols )) ), _rowOffset ( 0 ), _colOffset ( 0 )  {
    cerr << aol::color::error << "TripletMatrixOffset: This constructor is not useful for this offset wrapper class. Not implemented!" << aol::color::reset << endl;
  }

  //! \brief Constructor.
  //! \param[in] mat The matrix.
  //! \param[in] numRows The number of rows of the block.
  //! \param[in] numCols The number of columns of the block.
  //! \param[in] rowOffset The row (in the full matrix mat) at which the block starts.
  //! \param[in] colOffset The column in the full matrix mat) at which the block starts.
  explicit TripletMatrixOffset ( TripletMatrix<DataType> &mat, int numRows, int numCols,
      const int rowOffset, const int colOffset )
  : Matrix<DataType> ( numRows, numCols ), _mat ( mat ), _rowOffset ( rowOffset ), _colOffset ( colOffset ) {}

  //! \brief Copy constructor.
  explicit TripletMatrixOffset ( const TripletMatrixOffset &other, CopyFlag copyFlag = DEEP_COPY )
  : Matrix<DataType> ( other.getNumRows (), other.getNumCols () ), _mat ( other._mat ) {
    switch ( copyFlag ) {
    case DEEP_COPY:
      _rowOffset = other._rowOffset;
      _colOffset = other._colOffset;
      break;
    default:
      throw aol::UnimplementedCodeException ( "Copy flag not implemented", __FILE__, __LINE__ );
      break;
    }
  }

  //! \brief Destructor.
  virtual ~TripletMatrixOffset () {}

  //! \brief Returns entry (row, col) of the block.
  virtual DataType get ( int row, int col ) const {
    return _mat.get ( row + _rowOffset, col + _colOffset );
  }

  //! \brief Get reference to underlying TripletMatrix
  TripletMatrix<DataType> & getTripletMatrixRef( ) const  {
    return _mat;
  }

  //! \brief Get row offset
  inline int getRowOffset( ) const  {
    return _rowOffset;
  }

  //! \brief Get col offset
  inline int getColOffset( ) const  {
    return _colOffset;
  }

  //! \brief Sets entry (row, col) of the block to value.
  virtual void set ( int row, int col, DataType value ) {
    _mat.set ( row + _rowOffset, col + _colOffset, value );
  }

  //! \brief Adds value to entry (row, col) of the block.
  virtual void add ( int row, int col, DataType value ) {
    _mat.add ( row + _rowOffset, col + _colOffset, value );
  }

  //! \brief Sets the block to zero.
  //! \attention setZero on this class is very slow!
  virtual void setZero () {
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      for ( int j = 0; j < this->getNumCols (); ++j ) {
        _mat.eraseValue ( i + _rowOffset, j + _colOffset );
      }
    }
  }

  //! \brief Remove the i-th row and column of the block.
  void removeRowCol ( unsigned int i ) {
    if ( _rowOffset != _colOffset )
      throw aol::Exception ( "Only supported if rowOffset == colOffset", __FILE__, __LINE__, __FUNCTION__ );

    _mat.removeRowCol ( i + _rowOffset );
  }

  //! \brief Set the i-th row to zero
  void setRowToZero ( int i ) {
    for ( int k = 0; k < _mat._rowIndex.size (); ++k ) {
      if ( _mat._rowIndex[k] == i + _rowOffset && _mat._rowIndex[k] >= _rowOffset
          && _mat._rowIndex[k] < _rowOffset + this->getNumRows () ) {
        _mat._value[k] = 0.0;
      }
    }
  }

  //! \brief Set the i-th column to zero
  void setColToZero ( int j ) {
    for ( int k = 0; k < _mat._colIndex.size (); ++k ) {
      if ( _mat._colIndex[k] == j + _colOffset && _mat._colIndex[k] >= _colOffset
          && _mat._colIndex[k] < _colOffset + this->getNumCols () ) {
        _mat._value[k] = 0.0;
      }
    }
  }

  //! \brief Apply function for compatibility with other matrices.
  //! \warning Not implemented.
  virtual void apply ( const aol::Vector<DataType> &, aol::Vector<DataType> & ) const {
    throw aol::UnimplementedCodeException ( "Apply not implemented...", __FILE__, __LINE__ );
  }

  //! \brief applyAdd method for compatibility with other matrices.
  //! \warning Not implemented.
  virtual void applyAdd ( const aol::Vector<DataType> &, aol::Vector<DataType> & ) const {
    throw aol::UnimplementedCodeException ( "ApplyAdd not implemented...", __FILE__, __LINE__ );
  }
};

/**
 * \brief  Special TripletMatrixOffset wrapper
 * \author geihe
 *
 * Fills only the upper triangular part of the specified block, all other write requests are discarded
 */
template <typename DataType>
class TripletMatrixOffsetUpperTriangle : public TripletMatrixOffset<DataType> {
public:
  //! \brief Standard constructor. Create matrix of size (0, 0).
  explicit TripletMatrixOffsetUpperTriangle ()
  : TripletMatrixOffset<DataType>()
  {}

  //! \brief Constructor.
  //! \param[in] mat The matrix.
  //! \param[in] numRows The number of rows of the block.
  //! \param[in] numCols The number of columns of the block.
  //! \param[in] rowOffset The row (in the full matrix mat) at which the block starts.
  //! \param[in] colOffset The column in the full matrix mat) at which the block starts.
  explicit TripletMatrixOffsetUpperTriangle ( TripletMatrix<DataType> &mat, int numRows, int numCols,
                                              const int rowOffset, const int colOffset )
  : TripletMatrixOffset<DataType> ( mat, numRows, numCols, rowOffset, colOffset )
  {}

  //! \brief Destructor.
  virtual ~TripletMatrixOffsetUpperTriangle () {}

  //! \brief Copy constructor.
  explicit TripletMatrixOffsetUpperTriangle ( const TripletMatrixOffsetUpperTriangle &other, CopyFlag copyFlag = DEEP_COPY )
  : TripletMatrixOffset<DataType> ( other, copyFlag )  {}

  //! \brief Adds value to entry (row, col) of the block only if within upper triangular part of the block
  virtual void add ( int row, int col, DataType value ) {
    // only the upper triangular part should be filled, filter out the rest
    if ( row <= col ) this->_mat.add ( row + this->_rowOffset, col + this->_colOffset, value );
  }
};

//! \brief Base class for compressed sparse (row, column) matrices
//! \author Toelkes
template <typename DataType, typename IndexType>
class CSMatrix : public Matrix<DataType> {
protected:
  aol::Vector<IndexType> _index;
  aol::Vector<IndexType> _indPointer;
  aol::Vector<DataType>  _value;

public:
  explicit CSMatrix ()
  : Matrix<DataType> ( 0, 0 ) {}

  explicit CSMatrix ( IndexType numRows, IndexType numCols )
  : Matrix<DataType> ( numRows, numCols ), _indPointer ( numRows + 1 ) {}

  explicit CSMatrix ( const CSMatrix<DataType, IndexType> &other, CopyFlag copyFlag = DEEP_COPY )
  : Matrix<DataType> ( other, copyFlag ), _index ( other._index, copyFlag ),
    _indPointer ( other._indPointer, copyFlag ), _value ( other._value, copyFlag ) {
    switch ( copyFlag ) {
    case DEEP_COPY:
    case FLAT_COPY:
      // Nothing more to do.
      break;
    case STRUCT_COPY:
      // Set size to zero, s.t. all methods work as expected.
      this->_index.resize ( 0 );
      this->_indPointer.resize ( 0 );
      this->_value.resize ( 0 );
      break;
    default:
      throw aol::UnimplementedCodeException ( "Copy flag not implemented", __FILE__, __LINE__ );
      break;
    }
  }
};

/**
 * \brief A compressed sparse column matrix.
 * \attention Changes to the sparsity structure of the matrix (via add, set, ...) are slow!
 * \warning If IndexType is set to a type bigger than int (or unsigned int), triplet to csc conversion does no longer work (see CSCMatrix::setFromTriplet).
 * \author Toelkes
 *
 * A compressed sparse column matrix. Fast for matrix-vector multiplication and certain arithmetic operations.
 * Changes to the sparsity structure are slow. Use the TripletMatrix for assembling and then convert it into a CSCMatrix.
 */
template <typename DataType, typename IndexType = int>
class CSCMatrix : public CSMatrix<DataType, IndexType> {
  // In this class, _index lists row indices and _indPointer holds column pointers.
protected:
  void setFromTriplet ( const TripletMatrix<DataType> &tripletMatrix ) {
    const Vector<int> &tripletRow = tripletMatrix.getRowIndexReference ();
    const Vector<int> &tripletCol = tripletMatrix.getColIndexReference ();
    const Vector<DataType> &tripletVal = tripletMatrix.getValueReference ();

    // helper variables, make t long int, because it needs to be signed. If IndexType = unsigned long int, this does not work!
    aol::Vector<int64_t> t ( aol::Max ( this->getNumRows (), this->getNumCols () ) );
    int k;

    // Convert tripletMatrix into a (compressed) row matrix with duplicate entries and unsorted rows:
    aol::Vector<IndexType> rowPointer ( this->getNumRows () + 1 );
    aol::Vector<IndexType> colIndex ( tripletCol.size () );
    aol::Vector<DataType> val ( tripletVal.size () );

    // Count num. of entries per row
    for ( IndexType i = 0; i < tripletVal.size (); ++i ) {
      ++( t[tripletRow[i]] );
    }

    // Set row pointers
    for ( int j = 0; j < this->getNumCols (); ++j ) {
      rowPointer[j + 1] = rowPointer[j] + t[j];
      t[j] = rowPointer[j];
    }

    // Fill matrix
    for ( int i = 0; i < tripletVal.size (); ++i ) {
      k = ( t[tripletRow[i]] )++;
      colIndex[k] = tripletCol[i];
      val[k] = tripletVal[i];
    }

    t.setAll ( -1 );

    IndexType p1;
    IndexType p2;
    IndexType pd;
    IndexType pj;
    aol::Vector<IndexType> rowCount ( this->getNumRows () );

    // Sum up duplicate entries
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      p1 = rowPointer[i];
      p2 = rowPointer[i + 1];
      pd = p1;
      for ( IndexType p = p1; p < p2; ++p ) {
        k = colIndex[p];
        pj = t[k];

        // New entry
        if ( t[k] < p1 ) {
          t[k] = pd;
          colIndex[pd] = k;
          val[pd] = val[p];
          ++pd;
        }
        // Entry already seen.
        else {
          val[pj] += val[p];
        }
      }

      rowCount[i] = pd - p1;
    }

    t.setZero ();

    // Convert into a compressed column form matrix
    // Count num. of entries per column.
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      for ( IndexType p = rowPointer[i]; p < rowPointer[i] + rowCount[i]; ++p ) {
        ++( t[colIndex[p]] );
      }
    }

    IndexType numEntries = 0;
    for ( int i = 0; i < t.size (); ++i )
      numEntries += t[i];

    this->_index.resize ( numEntries );
    this->_value.resize ( numEntries );

    // Set column pointers
    this->_indPointer.resize ( this->getNumCols () + 1);
    this->_indPointer[0] = static_cast<IndexType> ( 0 );
    for ( int j = 0; j < this->getNumCols (); ++j ) {
      this->_indPointer[j + 1] = this->_indPointer[j] + t[j];
      t[j] = this->_indPointer[j];
    }

    // Fill matrix
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      for ( IndexType p = rowPointer[i]; p < rowPointer[i] + rowCount[i]; ++p ) {
        k = ( t[colIndex[p]] )++;
        this->_index[k] = i;
        this->_value[k] = val[p];
      }
    }
  }

  //! \warning Untested!
  void setFromSparse ( const SparseMatrix<DataType> &sparseMatrix ) {
    // helper variables, make t long int, because it needs to be signed. If IndexType = unsigned long int, this does not work!
    aol::Vector<long int> t ( aol::Max ( this->getNumRows (), this->getNumCols () ) );
    int k;

    // Convert into a compressed column form matrix
    // Count num. of entries per column.
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      for ( aol::SparseMatrixRowIterator<DataType> it ( sparseMatrix, i ); it.notAtEnd (); ++it ) {
        ++( t[it->col] );
      }
    }

    IndexType numEntries = 0;
    for ( int i = 0; i < t.size (); ++i )
      numEntries += t[i];

    this->_index.resize ( numEntries );
    this->_value.resize ( numEntries );

    // Set column pointers
    this->_indPointer.resize ( this->getNumCols () + 1);
    this->_indPointer[0] = static_cast<IndexType> ( 0 );
    for ( int j = 0; j < this->getNumCols (); ++j ) {
      this->_indPointer[j + 1] = this->_indPointer[j] + t[j];
      t[j] = this->_indPointer[j];
    }

    // Fill matrix
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      for ( aol::SparseMatrixRowIterator<DataType> it ( sparseMatrix, i ); it.notAtEnd (); ++it ) {
        k = ( t[it->col] )++;
        this->_index[k] = i;
        this->_value[k] = it->val;
      }
    }
  }

public:
  //! \brief Constructor taking the number of rows and columns.
  explicit CSCMatrix ( IndexType numRows, IndexType numCols )
  : CSMatrix<DataType, IndexType> ( static_cast<int> ( numRows ), static_cast<int> ( numCols ) ) {}

  //! \brief Standard constructor.
  explicit CSCMatrix ()
  : CSMatrix<DataType, IndexType> ( 0, 0 ) {}

  //! \brief Copy constructor.
  CSCMatrix ( const CSCMatrix &other, CopyFlag copyFlag = DEEP_COPY )
  : CSMatrix<DataType, IndexType> ( other, copyFlag ) {}

  /** \brief Constructor that converts a TripletMatrix into a CSCMatrix.
   *
   * Converts tripletMatrix into a CSCMatrix. If the TripletMatrix has duplicate
   * entries, the contributions are summed up (so it is not necessary
   * to call sumDuplicates on the TripletMatrix).
   */
  CSCMatrix ( const TripletMatrix<DataType> &tripletMatrix )
  : CSMatrix<DataType, IndexType> ( tripletMatrix.getNumRows (), tripletMatrix.getNumCols () ) {
    setFromTriplet ( tripletMatrix );
  }

  //! \brief Destructor.
  virtual ~CSCMatrix () {}

  //! \brief Set the matrix to zero.
  virtual void setZero () {
    this->_index.reallocateClear ( 0 );
    this->_indPointer.reallocateClear ( 0 );
    this->_value.reallocateClear ( 0 );
  }

  CSCMatrix& operator= ( const TripletMatrix<DataType> &tripletMatrix ) {
    this->setZero ();
    this->_numRows = tripletMatrix.getNumRows ();
    this->_numCols = tripletMatrix.getNumCols ();
    setFromTriplet ( tripletMatrix );

    return *this;
  }

  //! \brief Get matrix entry (row, col).
  virtual DataType get ( int row, int col ) const {
    // Look for an entry in the column. The end of column col is this->_indPointer[col + 1]
    for ( IndexType i = this->_indPointer[col]; i < this->_indPointer[col + 1]; ++i ) {
      if ( this->_index[i] == static_cast<IndexType> ( row ) ) {
        return this->_value[i];
      }
    }

    // If no value has been found, the entry is zero.
    return static_cast<DataType> ( 0 );
  }

  //! \brief Sets entry (row, col) of the block to value.
  //! \attention Changes to the sparsity structure of the matrix are (very) slow!
  virtual void set ( int row, int col, DataType val ) {
    // Look for an entry in the column. The end of column col is this->_indPointer[col + 1].
    for ( IndexType i = this->_indPointer[col]; i < this->_indPointer[col + 1]; ++i ) {
      // If the entry already exists, set it and exit the method.
      if ( this->_index[i] == static_cast<IndexType> ( row ) ) {
        this->_value[i] = val;
        return;
      }
    }

    // If no value has been found, insert a new entry.
    // Find position to insert new value
    IndexType pos = this->_indPointer[col];
    while ( pos < this->_indPointer[col + 1] ) {
      if ( this->_index[pos] > row )
        break;

      ++pos;
    }

    // And insert new value at this position
    this->_index.insert ( pos, row );
    this->_value.insert ( pos, val );

    // Following column pointers have to be shifted by 1
    for ( IndexType c = col + 1; c < static_cast<IndexType> ( this->_indPointer.size () ); ++c ) {
      ++( this->_indPointer[c] );
    }
  }

  //! \brief Add value to entry (row, col).
  //! \attention Changes to the sparsity structure of the matrix are (very) slow!
  virtual void add ( int row, int col, DataType val ) {
    // Look for an entry in the column. The end of column col is this->_indPointer[col + 1].
    for ( IndexType i = this->_indPointer[col]; i < this->_indPointer[col + 1]; ++i ) {
      // If the entry already exists, set it and exit the method.
      if ( this->_index[i] == static_cast<IndexType> ( row ) ) {
        this->_value[i] += val;
        return;
      }
    }

    // If no value has been found, insert a new entry.
    // Find position to insert new value
    IndexType pos = this->_indPointer[col];
    while ( pos < this->_indPointer[col + 1] ) {
      if ( this->_index[pos] > row )
        break;

      ++pos;
    }

    // And insert new value at this position
    this->_index.insert ( pos, row );
    this->_value.insert ( pos, val );

    // Following column pointers have to be shifted by 1
    for ( IndexType c = col + 1; c < static_cast<IndexType> ( this->_indPointer.size () ); ++c ) {
      ++( this->_indPointer[c] );
    }
  }

  //! \brief applyAdd method.
  virtual void applyAdd ( const aol::Vector<DataType> &arg, aol::Vector<DataType> &dest ) const {
    // Traverse the matrix column-wise, as this is faster for the CSCMatrix.
    for ( int col = 0; col < this->getNumCols (); ++col ) {
      for ( IndexType i = this->_indPointer[col]; i < this->_indPointer[col + 1]; ++i ) {
        dest[this->_index[i]] += ( this->_value[i] * arg[col] );
      }
    }
  }

  //! \brief apply method.
  virtual void apply ( const aol::Vector<DataType> &arg, aol::Vector<DataType> &dest ) const {
    dest.setZero ();

    // Traverse the matrix column-wise, as this is faster for the CSCMatrix.
    for ( int col = 0; col < this->getNumCols (); ++col ) {
      for ( IndexType i = this->_indPointer[col]; i < this->_indPointer[col + 1]; ++i ) {
        dest[this->_index[i]] += ( this->_value[i] * arg[col] );
      }
    }
  }

  void applyAdd ( const aol::MultiVector<DataType> &arg, aol::MultiVector<DataType> &dest ) const {
    aol::Vector<DataType> rhs( arg.getTotalSize () ), sol( dest.getTotalSize () );
    rhs.copyUnblockedFrom ( arg );
    sol.copyUnblockedFrom ( dest );

    this->applyAdd ( rhs, sol );

    dest.copySplitFrom ( sol );
  }

  void apply ( const aol::MultiVector<DataType> &arg, aol::MultiVector<DataType> &dest ) const {
    aol::Vector<DataType> rhs( arg.getTotalSize () ), sol( dest.getTotalSize () );
    rhs.copyUnblockedFrom ( arg );

    this->apply ( rhs, sol );

    dest.copySplitFrom ( sol );
  }

  const aol::Vector<IndexType>& getRowIndexReference () const {
    return this->_index;
  }

  const aol::Vector<IndexType>& getColumnPointerReference () const {
    return this->_indPointer;
  }

  const aol::Vector<DataType>& getValueReference () const {
    return this->_value;
  }
};

/**
 * \brief A compressed sparse row matrix.
 * \attention Changes to the sparsity structure of the matrix (via add, set, ...) are slow!
 * \warning If IndexType is set to a type bigger than int (or unsigned int), triplet to csr conversion does no longer work (see CSCMatrix::setFromTriplet).
 * \author Toelkes
 *
 * A compressed sparse row matrix. Fast for matrix-vector multiplication and certain arithmetic operations.
 * Changes to the sparsity structure are slow. Use the TripletMatrix for assembling and then convert it into a CSRMatrix.
 */
template <typename DataType, typename IndexType = int>
class CSRMatrix : public CSMatrix<DataType, IndexType> {
  // In this class, _index lists column indices and _indPointer holds row pointers.
protected:
  void setFromTriplet ( const TripletMatrix<DataType> &tripletMatrix ) {
    const Vector<int> &tripletRow = tripletMatrix.getRowIndexReference ();
    const Vector<int> &tripletCol = tripletMatrix.getColIndexReference ();
    const Vector<DataType> &tripletVal = tripletMatrix.getValueReference ();

    // helper variables, make t long int, because it needs to be signed. If IndexType = unsigned long int, this does not work!
    aol::Vector<int64_t> t ( aol::Max ( this->getNumRows (), this->getNumCols () ) );
    int k;

    // Convert tripletMatrix into a (compressed) row matrix with duplicate entries and unsorted rows:
    aol::Vector<IndexType> rowIndex ( tripletRow.size () );
    aol::Vector<IndexType> colPointer ( this->getNumCols () + 1 );
    aol::Vector<DataType> val ( tripletVal.size () );

    // Count num. of entries per row
    for ( IndexType j = 0; j < tripletVal.size (); ++j ) {
      ++( t[tripletCol[j]] );
    }

    // Set column pointers
    for ( int i = 0; i < this->getNumRows (); ++i ) {
      colPointer[i + 1] = colPointer[i] + t[i];
      t[i] = colPointer[i];
    }

    // Fill matrix
    for ( int i = 0; i < tripletVal.size (); ++i ) {
      k = ( t[tripletCol[i]] )++;
      rowIndex[k] = tripletRow[i];
      val[k] = tripletVal[i];
    }

    t.setAll ( -1 );

    IndexType p1;
    IndexType p2;
    IndexType pd;
    IndexType pj;
    aol::Vector<IndexType> colCount ( this->getNumRows () );

    // Sum up duplicate entries
    for ( int j = 0; j < this->getNumCols (); ++j ) {
      p1 = colPointer[j];
      p2 = colPointer[j + 1];
      pd = p1;
      for ( IndexType p = p1; p < p2; ++p ) {
        k = rowIndex[p];
        pj = t[k];

        // New entry
        if ( t[k] < p1 ) {
          t[k] = pd;
          rowIndex[pd] = k;
          val[pd] = val[p];
          ++pd;
        }
        // Entry already seen.
        else {
          val[pj] += val[p];
        }
      }

      colCount[j] = pd - p1;
    }

    t.setZero ();

    // Convert into a compressed row form matrix
    // Count num. of entries per row.
    for ( int j = 0; j < this->getNumRows (); ++j ) {
      for ( IndexType p = colPointer[j]; p < colPointer[j] + colCount[j]; ++p ) {
        ++( t[rowIndex[p]] );
      }
    }

    IndexType numEntries = 0;
    for ( int i = 0; i < t.size (); ++i )
      numEntries += t[i];

    this->_index.resize ( numEntries );
    this->_value.resize ( numEntries );

    // Set column pointers
    this->_indPointer.resize ( this->getNumRows () + 1);
    this->_indPointer[0] = static_cast<IndexType> ( 0 );
    for ( int i = 0; i < this->getNumCols (); ++i ) {
      this->_indPointer[i + 1] = this->_indPointer[i] + t[i];
      t[i] = this->_indPointer[i];
    }

    // Fill matrix
    for ( int j = 0; j < this->getNumCols (); ++j ) {
      for ( IndexType p = colPointer[j]; p < colPointer[j] + colCount[j]; ++p ) {
        k = ( t[rowIndex[p]] )++;
        this->_index[k] = j;
        this->_value[k] = val[p];
      }
    }
  }

public:
  //! \brief Constructor taking the number of rows and columns.
  CSRMatrix ( IndexType numRows, IndexType numCols )
  : CSMatrix<DataType, IndexType>  ( static_cast<int> ( numRows ), static_cast<int> ( numCols ) ) {}

  //! \brief Standard constructor.
  CSRMatrix ()
  : CSMatrix<DataType, IndexType> ( 0, 0 ) {}

  //! \brief Copy constructor.
  CSRMatrix ( const CSRMatrix &other, CopyFlag copyFlag = DEEP_COPY )
  : CSMatrix<DataType, IndexType> ( other, copyFlag ) {}

  /** \brief Constructor that converts a TripletMatrix into a CSRMatrix.
   *
   * Converts tripletMatrix into a CSRMatrix. If the TripletMatrix has duplicate
   * entries, the contributions are summed up (so it is not necessary
   * to call sumDuplicates on the TripletMatrix).
   */
  CSRMatrix ( const TripletMatrix<DataType> &tripletMatrix )
  : CSMatrix<DataType, IndexType> ( tripletMatrix.getNumRows (), tripletMatrix.getNumCols () ) {
    setFromTriplet ( tripletMatrix );
  }

  //! \brief Destructor.
  virtual ~CSRMatrix () {}

  //! \brief Set the matrix to zero.
  virtual void setZero () {
    this->_index.reallocateClear ( 0 );
    this->_indPointer.reallocateClear ( 0 );
    this->_value.reallocateClear ( 0 );
  }

  CSRMatrix& operator= ( const TripletMatrix<DataType> &tripletMatrix ) {
    this->setZero ();
    this->_numRows = tripletMatrix.getNumRows ();
    this->_numCols = tripletMatrix.getNumCols ();
    setFromTriplet ( tripletMatrix );

    return *this;
  }

  //! \brief Get matrix entry (row, col).
  virtual DataType get ( int row, int col ) const {
    // Look for an entry in the row. The end of row row is this->_indPointer[row + 1]
    for ( IndexType j = this->_indPointer[row]; j < this->_indPointer[row + 1]; ++j ) {
      if ( this->_index[j] == static_cast<IndexType> ( col ) ) {
        return this->_value[j];
      }
    }

    // If no value has been found, the entry is zero.
    return static_cast<DataType> ( 0 );
  }

  //! \brief Sets entry (row, col) of the matrix to value.
  //! \attention Changes to the sparsity structure of the matrix are (very) slow!
  virtual void set ( int row, int col, DataType val ) {
    // Look for an entry in the row. The end of row row is this->_indPointer[row + 1]
    for ( IndexType j = this->_indPointer[row]; j < this->_indPointer[row + 1]; ++j ) {
      // If the entry already exists, set it and exit the method.
      if ( this->_index[j] == static_cast<IndexType> ( col ) ) {
        this->_value[j] = val;
        return;
      }
    }

    // If no value has been found, insert a new entry.
    // Find position to insert new value
    IndexType pos = this->_indPointer[row];
    while ( pos < this->_indPointer[row + 1] ) {
      if ( this->_index[pos] > col )
        break;

      ++pos;
    }

    // And insert new value at this position
    this->_index.insert ( pos, col );
    this->_value.insert ( pos, val );

    // Following column pointers have to be shifted by 1
    for ( IndexType c = row + 1; c < static_cast<IndexType> ( this->_indPointer.size () ); ++c ) {
      ++( this->_indPointer[c] );
    }
  }

  //! \brief Add value to entry (row, col).
  //! \attention Changes to the sparsity structure of the matrix are (very) slow!
  virtual void add ( int row, int col, DataType val ) {
    // Look for an entry in the row. The end of row row is this->_indPointer[row + 1]
    for ( IndexType j = this->_indPointer[row]; j < this->_indPointer[row + 1]; ++j ) {
      // If the entry already exists, set it and exit the method.
      if ( this->_index[j] == static_cast<IndexType> ( col ) ) {
        this->_value[j] += val;
        return;
      }
    }

    // If no value has been found, insert a new entry.
    // Find position to insert new value
    IndexType pos = this->_indPointer[row];
    while ( pos < this->_indPointer[row + 1] ) {
      if ( this->_index[pos] > col )
        break;

      ++pos;
    }

    // And insert new value at this position
    this->_index.insert ( pos, col );
    this->_value.insert ( pos, val );

    // Following column pointers have to be shifted by 1
    for ( IndexType c = row + 1; c < static_cast<IndexType> ( this->_indPointer.size () ); ++c ) {
      ++( this->_indPointer[c] );
    }
  }

  //! \brief applyAdd method.
  virtual void applyAdd ( const aol::Vector<DataType> &arg, aol::Vector<DataType> &dest ) const {
    // Traverse the matrix row-wise.
    for ( int row = 0; row < this->getNumRows (); ++row ) {
      for ( IndexType j = this->_indPointer[row]; j < this->_indPointer[row + 1]; ++j ) {
        dest[row] += ( this->_value[j] * arg[this->_index[j]] );
      }
    }
  }

  //! \brief apply method.
  virtual void apply ( const aol::Vector<DataType> &arg, aol::Vector<DataType> &dest ) const {
    // Traverse the matrix row-wise.
    for ( int row = 0; row < this->getNumRows (); ++row ) {
      DataType s = static_cast<DataType> ( 0 );
      for ( IndexType j = this->_indPointer[row]; j < this->_indPointer[row + 1]; ++j ) {
        s += ( this->_value[j] * arg[this->_index[j]] );
      }
      dest[row] = s;
    }
  }

  void applyAdd ( const aol::MultiVector<DataType> &arg, aol::MultiVector<DataType> &dest ) const {
    aol::Vector<DataType> rhs( arg.getTotalSize () ), sol( dest.getTotalSize () );
    rhs.copyUnblockedFrom ( arg );
    sol.copyUnblockedFrom ( dest );

    this->applyAdd ( rhs, sol );

    dest.copySplitFrom ( sol );
  }

  void apply ( const aol::MultiVector<DataType> &arg, aol::MultiVector<DataType> &dest ) const {
    aol::Vector<DataType> rhs( arg.getTotalSize () ), sol( dest.getTotalSize () );
    rhs.copyUnblockedFrom ( arg );

    this->apply ( rhs, sol );

    dest.copySplitFrom ( sol );
  }

  const aol::Vector<IndexType>& getRowPointerReference () const {
    return this->_indPointer;
  }

  const aol::Vector<IndexType>& getColumnIndexReference () const {
    return this->_index;
  }

  const aol::Vector<DataType>& getValueReference () const {
    return this->_value;
  }
};

}

#endif
