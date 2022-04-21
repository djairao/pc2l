// Copyright 2021 JD Rudie
#ifndef MATRIX_H
#define MATRIX_H

/** A simple, yet versatile 2-D Matrix class
 *
 */

#include <iostream>
#include <functional>
#include "pc2l.h"
#include <cassert>
#include <gtest/gtest.h>

/** Shortcut for the value of each element in the matrix */
using Val = double;

/** Short cut to a 2-d vector double values to streamline the code */
using Vec = pc2l::Vector<Val>;

/** A matrix class to perform basic matrix operations.

    The class essentially encapsulates a 2-d matrix of double values
    and performs the following matrix operations:

    <ul>
    <li>Create a matrix of given dimensions.</li>

    <li>Matrix multiplication using Block matrix multiplication.</li>

    <li> Stream insertion and extraction operators to conveniently
    load and print values.</li>

    </ul>
*/
class Matrix : public Vec {
    /** Stream insertion operator to ease printing matrices
     *
     * This method prints the dimension of the matrix and then prints
     * the values in a row-by-row manner.  The output is setup to be
     * consistent with the stream extraction operator.
     *
     * \param[out] os The output stream to where the data is to be
     * written. This could be any output stream -- for example,
     * a std::ofstream, std::ostringstream, or std::cout.
     *
     * \param[in] matrix The matrix to be written.
     *
     * \return As per convention, this method returns the supplied
     * output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

    /** Stream extraction operator to ease reading matrices
     *
     * This method reads the dimension of the matrix and then reads
     * the values in a row-by-row manner.  The input is assumed to be
     * that one generated by the stream insertion operator.
     *
     * \param[in/out] in The input stream to where the data is to be
     * read. This could be any input stream -- for example,
     * a std::ifstream, std::istringstream, or std::cin.
     *
     * \param[in] matrix The matrix to be read.
     *
     * \return As per convention, this method returns the supplied
     * input stream.
     */
    friend std::istream& operator>>(std::istream& is, Matrix& matrix);

    size_t rows;

    size_t cols;

public:
    /**
     * Constructor to create and initialize a matrix.
     *
     * \param[in] rows The number of rows to be created in the
     * matrix.
     *
     * \param[in] cols The number of cols to be created in the matrix.
     *
     * \param[in] initVal The inital value to be set for each entry in
     * the matrix.
     */
    explicit Matrix(const size_t rows = 0, const size_t cols = 0,
                    const Val initVal = 0);

    /**
     * Returns the height or number of rows in this matrix.
     *
     * \return Returns the height or number of rows in this matrix.
     */
    size_t height() const { return rows; };

    /**
     * Returns the width or number of columns in this matrix.
     *
     * \return Returns the width or number of columns in this matrix.
     */
    size_t width() const { return cols; };

    /**
     * Creates a new matrix in which each value is obtained by
     * applying a given unary operator to each entry in the matrix.
     *
     * \param[in] operation The unary operation to be used to create
     * the given matrix.
     */
    template<typename UnaryOp>
    Matrix apply(const UnaryOp& operation) const {
        Matrix ret(this->height(), this->width());
        for (size_t row = 0; row < ret.height(); row++) {
            for (size_t col = 0; col < ret.width(); col++) {
//                ret[row][col] = operation(this->at(row)[col])
                ret.insert(row, col, operation(at(row, col)));
            }
        }
        return ret;
    }

    /**
     * Convenience method for inserting the value into matrix cell (\p row, \p cell)
     * @param row row of cell to insert into
     * @param col column number of cell to insert into
     * @param val value to insert into cell
     */
    void insert(size_t row, size_t col, Val val) {
        pc2l::Vector<Val>::insert(row * rows + col, val);
    };

    /**
     * Convenience method for replacing the value in matrix cell (\p row, \p cell)
     * @param row row of cell to replace
     * @param col column number of cell to replace
     * @param val value to replace into cell
     */
    void replace(size_t row, size_t col, Val val) {
        pc2l::Vector<Val>::replace(row * rows + col, val);
    };

    /**
     * Convenience method for erasing the value in matrix cell (\p row, \p cell)
     * @param row row of cell to replace
     * @param col column number of cell to replace
     * @param val value to replace into cell
     */
    void erase(size_t row, size_t col) {
        pc2l::Vector<Val>::erase(row * rows + col);
    };

    /**
     * Convenience method for getting the value in matrix cell (\p row, \p col).
     * @param row the row number of cell, 0 indexed
     * @param col column number of cell, 0 indexed
     * @return the data at (\p row, \p col)
     */
    Val at(size_t row, size_t col) const {
        return pc2l::Vector<Val>::at(row * width() + col);
    }

    /**
     * Operator to add two matrices with the same dimensions together.
     *
     * \param[in] rhs The other matrix to be used.  This matrix must
     * have the same dimension as this matrix.  Otherwise this method
     * throws an excpetion.
     *
     * \return The resulting matrix in which each value has been
     * computed by adding the corresponding values from \c this and
     * rhs.
     */
    Matrix operator+(const Matrix& rhs) const;

    /**
     * Operator for computing the Hadamard product of two matrices
     * with the same dimensions.
     *
     * \param[in] rhs The other matrix to be used.  This matrix must
     * have the same dimension as this matrix.  Otherwise this method
     * throws an excpetion.
     *
     * \return The resulting matrix in which each value has been
     * computed by multiplying the corresponding values from \c this
     * and rhs.
     */
    Matrix operator*(const Matrix& rhs) const;

    /**
     * Operator for computing the Hadamard product of two matrices
     * with the same dimensions.
     *
     * \param[in] rhs The other matrix to be used.  This matrix must
     * have the same dimension as this matrix.  Otherwise this method
     * throws an excpetion.
     *
     * \return The resulting matrix in which each value has been
     * computed by multiplying the corresponding values from \c this
     * and rhs.
     */
    Matrix operator*(const Val val) const;

    /**
     * Operator to subtract two matrices with the same dimensions.
     *
     * \param[in] rhs The other matrix to be used.  This matrix must
     * have the same dimension as this matrix.  Otherwise this method
     * throws an excpetion.
     *
     * \return The resulting matrix in which each value has been
     * computed by subtracting the corresponding values from \c this
     * and rhs.
     */
    Matrix operator-(const Matrix& rhs) const;

    /**
     * Performs the dot product of two matrices. This method has a
     * O(n^3) time complexity.
     *
     * \param[in] rhs The other matrix to be used.  This matrix must
     * have the same number of rows as the number of columns in this
     * matrix.  Otherwise this method throws an excpetion.
     *
     * \return The resulting matrix in which each value has been
     * computed by multiplying the corresponding values from \c this
     * and rhs.
     */
    Matrix dot(const Matrix& rhs) const;

    /**
     * Returns the transpose of this matrix.
     */
    Matrix transpose() const;
};

#endif

