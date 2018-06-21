#include <iostream>
#include <iomanip>
#include <fstream>
using namespace std;
//     max   rows  columns
const int M = 20, N = 20;

struct Tableau {
	int rows, columns;
	double mat[M][N];
};

void ReadTableau(Tableau *tab, const char* filename);
void PrintTableau(Tableau *tab, const char* message);
void DoPivoting(Tableau *tab, int row, int column);
void FindPivotColAndRow(Tableau *tab, int &row, int &column);
void AddSlackVariables(Tableau *tab);
 int FindBasisVariable(Tableau *tab, int column);
void PrintOptimalVector(Tableau *tab, const char *message);
void Simplex(Tableau *tab);

int main()
{
	Tableau table;

	ReadTableau(&table, "simplex.txt");
	PrintTableau(&table, "from file");
	Simplex(&table);
	int a; cin >> a;
	return 0;
}
// Inputs tableau from file
void ReadTableau(Tableau *tab, const char *filename){
	ifstream input(filename);
	if (!input.is_open()) {
		cout << "File not open! " << endl;
	}
	else {
		cout << "Reading file: " << filename << endl;
		input >> tab->rows >> tab->columns;
		for (int i = 0; i < tab->rows; i++)
			for (int j = 0; j < tab->columns; j++)
				input >> tab->mat[i][j];
	}
	input.close();
}
// Print table and tags
void PrintTableau(Tableau *tab, const char* message) {
	static int counter = 1;
	cout << counter++ << ". Table " << message << endl; // counter increases every time this function is called
	cout << "===================================================================" << endl;
	cout << "col:    b[i]\t";
	for (int j = 1; j < tab->columns; j++) 
		cout << "x" << j << "  \t";
	cout << endl;
	for (int i = 0; i < tab->rows; i++) {
		if (i == tab->rows - 1)
			cout << "max: ";
		else
			cout << "b" << i << "   ";
		for (int j = 0; j < tab->columns; j++)
			cout << setprecision(3) << "  \t" << tab->mat[i][j];
		cout << endl;
	}
	cout << "===================================================================" << endl;
}
// Do pivoting on mat[row][col]
// Transforms pivot column into identity
void DoPivoting(Tableau *tab, int row, int column) {
	double pivot = tab->mat[row][column];

	for (int j = 0; j < tab->columns; j++) // divide row by pivot to make value 1
		tab->mat[row][j] /= pivot;

	for (int i = 0; i < tab->rows; i++) { // foreach remaining row i 
		double multiplier = tab->mat[i][column]; // values from pivotColumn
												 cout << "Multiplier: " << multiplier << endl;
		if (i != row) // foreach row that's not the pivot one
			for (int j = 0; j < tab->columns; j++) { // foreach column j
			// transform row by deducting multiplier * pivotrow[j] values from it
			// New value -= (Previous value in pivot column * New value in pivot row)
				tab->mat[i][j] -= multiplier * tab->mat[row][j];
				//cout << "i: " << i << " j: " << j << endl;
				//cout << multiplier * tab->mat[row][j] << endl;
			}
	}
}
// Finds column and row to pivot on
// sets column to -1 if last row is all positive
void FindPivotColAndRow(Tableau *tab, int &row, int &column){
	row = -1;
	column = 1; // skip 1st col
	double min = tab->mat[tab->rows - 1][column];
	// Find pivot column
	for (int i = 1; i < tab->columns; i++)	{
		if (tab->mat[tab->rows - 1][i] < min) { // Search last row
			min = tab->mat[tab->rows - 1][i];
			column = i;
		}
	}
	cout << "Most negative column in row max is x" << column << " = " << min << endl;
	if (min >= 0) {
		column = -1;
	}else { // Find pivot row if col was found
		double minRatio = -1;
		cout << "Ratios: A[row_i, 0] / A[row_i, " << column << "] = [ ";
		for (int i = 0; i < tab->rows - 1; i++) {
			double ratio = tab->mat[i][0] / tab->mat[i][column];
			cout << ratio << " ";
			if ((ratio > 0 && ratio < minRatio) || minRatio < 0) {
				minRatio = ratio;
				row = i;
			}
		}
		cout << "] " << endl;
		if (minRatio == -1) {
			row = -1;
			cout << "Minimum ratio is not positive" << endl;
		} else {
			cout << "Minimum positive ratio: " << minRatio << endl;
			cout << "Pivot = " << "[" << row << "][" << column << "]" << endl;
		}
	}
}
// Adds slack variables
void AddSlackVariables(Tableau *tab) {
	for (int i = 0; i < tab->rows; i++) {
		for (int j = 1; j < tab->rows; j++) {
			//cout << i + 1 << " == " << j << endl;
			tab->mat[i][j + tab->columns - 1] = (i + 1 == j);
		}
	}
	tab->columns += tab->rows - 1; // Increase column count by rows - 1
}
// Checks if column is part of identity matrix ( has one 1 and rest 0s)
// Returns -1 if not
int  FindBasisVariable(Tableau *tab, int column) {
	int xi = -1;
	for (int i = 0; i < tab->rows - 1; i++) { // foreach row
		if (tab->mat[i][column] == 1) {
			if (xi == -1)
				xi = i;   // found first '1', save row number
			else
				return -1; // found second '1', not an identity matrix
		}
		else if (tab->mat[i][column] != 0) {
			return -1; // not an identity matrix
		}
	}
	return xi;
}
// Finds base variable (see FindBasisVariable) and prints it
void PrintOptimalVector(Tableau *tab, const char *message) {
	int xi;
	int realVariableCount = tab->columns - tab->rows + 1;
	cout << message << " at";
	for (int j = 1; j < realVariableCount; j++) { // foreach real variable
		xi = FindBasisVariable(tab, j);
		if (xi != -1)
			cout << " x" << j << " = " << tab->mat[xi][0] << ",";
		else
			cout << " x" << j << " = 0,";
	}
	cout << endl;
}
// Do simplex algorithm for set amount of iterations
void Simplex(Tableau *tab) {
	int loop = 0;
	AddSlackVariables(tab);
	PrintTableau(tab, "with slack variables");
	while (loop++ <= 20) {
		int pivotColumn, pivotRow;
		FindPivotColAndRow(tab, pivotRow, pivotColumn);
		if (pivotColumn < 0) {
			cout << "Found optimal value = " << tab->mat[tab->rows - 1][0] << endl;
			PrintOptimalVector(tab, "Optimal vector");
			break;
		}else if (pivotRow < 0) {
			cout << "Unbounded solution " << endl;
			break;
		}else {
			cout << "Make variable x" << pivotColumn << " basic" << endl;
			cout << "Pivot = " << tab->mat[pivotRow][pivotColumn] << endl;
			DoPivoting(tab, pivotRow, pivotColumn);
			PrintTableau(tab, "after pivoting");
			PrintOptimalVector(tab, "basic feasible solution");
		}
	}
}
