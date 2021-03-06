//----------------------------------------------------------------------
// This file is part of PARSEC (http://physik.rwth-aachen.de/parsec)
// a parametrized simulation engine for cosmic rays.
//
// Copyright (C) 2011  Martin Erdmann, Peter Schiffer, Tobias Winchen
//                     RWTH Aachen University, Germany
// Contact: winchen@physik.rwth-aachen.de
//
//  This program is free software: you can redistribute it and/or
//  modify it under the terms of the GNU General Public License as
//  published by the Free Software Foundation, either version 3 of
//  the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program. If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------

#include "crpropa/magneticLens/ModelMatrix.h"
#include <ctime>

#include <Eigen/Core>
namespace crpropa 
{

void serialize(const string &filename, const ModelMatrixType& matrix)
{
	ofstream outfile(filename.c_str(), ios::binary);
	if (!outfile)
	{
		throw runtime_error("Can't write file: " + filename);
	}

	uint32_t C = 0;
	double val;

	C = (uint32_t) (matrix.nonZeros());
	outfile.write((char*) &C, sizeof(uint32_t));
	C = (uint32_t) (matrix.rows());
	outfile.write((char*) &C, sizeof(uint32_t));
	C = (uint32_t) (matrix.cols());
	outfile.write((char*) &C, sizeof(uint32_t));

	// serialize non zero elements
	for (size_t col_idx = 0; col_idx < matrix.cols(); ++col_idx)
	{
		for (ModelMatrixType::InnerIterator it(matrix,col_idx); it; ++it)
			{
				it.value();
				C = (uint32_t) it.row();
				outfile.write((char*) &C, sizeof(uint32_t));

				C = (uint32_t) it.col();
				outfile.write((char*) &C, sizeof(uint32_t));

				val = it.value();
				outfile.write((char*) &val, sizeof(double));
				if (outfile.fail())
				{
					throw runtime_error("Error writing file: " + filename);
				}
			}
	}

	outfile.close();
}


void deserialize(const string &filename, ModelMatrixType& matrix)
{
	ifstream infile(filename.c_str(), ios::binary);
	if (!infile)
	{
		throw runtime_error("Can't read file: " + filename);
	}

	uint32_t nnz, nRows, nColumns;
	infile.read((char*) &nnz, sizeof(uint32_t));
	infile.read((char*) &nRows, sizeof(uint32_t));
	infile.read((char*) &nColumns, sizeof(uint32_t));
	matrix.resize(nRows, nColumns);
	matrix.reserve(nnz);

	uint32_t row, column;
	double val;
	std::vector< Eigen::Triplet<double> > triplets;
	triplets.resize(nnz);
	for (size_t i = 0; i < nnz; i++)
	{
		infile.read((char*) &row, sizeof(uint32_t));
		infile.read((char*) &column, sizeof(uint32_t));
		infile.read((char*) &val, sizeof(double));
		//M(size1,size2) = val;
		triplets[i] = Eigen::Triplet<double>(row, column, val);
	}
	matrix.setFromTriplets(triplets.begin(), triplets.end());
	matrix.makeCompressed();
}


double norm_1(const ModelVectorType &v)
{
	return v.cwiseAbs().sum();
}


void normalizeColumns(ModelMatrixType &matrix){
	for (size_t i=0; i< matrix.cols(); i++)
	{
		ModelVectorType v = matrix.col(i);
		double rn = norm_1(v);
		matrix.col(i) = v/rn;
	}
}


double maximumOfSumsOfColumns(const ModelMatrixType &matrix) 
{
	double summax = 0;
	for (size_t i = 0; i < matrix.cols(); i++)
	{
		double sum = matrix.col(i).sum();
		if (sum > summax)
			summax = sum;
	}
	return summax;
}

	void normalizeMatrix(ModelMatrixType& matrix, double norm)
{
	matrix /= norm;
}

	void prod_up(const ModelMatrixType& matrix, double* model)
{

	// copy storage of model, as matrix vector product cannot be done
	// in place
	
	const size_t mSize = matrix.cols();
	double *origVectorStorage = new double[mSize];
	memcpy(origVectorStorage, model, mSize * sizeof(double));



	Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, 1> > origVectorAdaptor(origVectorStorage, mSize);
	Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, 1> > modelVectorAdaptor(model, mSize);

	// perform the optimized product
	modelVectorAdaptor = matrix * origVectorAdaptor;

	// clean up
	delete[] origVectorStorage;
}


} // namespace parsec
