//variable 'i' used to iterate through rows
//variable 'j' used to iterate through columns
#include <iostream>
#include <fstream>
#include <string>
static const int DIMENSION = 9; //DIMENSION of sudoku square
class PossibleValues{
	bool values[DIMENSION]; //bitset representing all possible values a particular square can have
public:
	//constructor. Defaults all values to true.
	PossibleValues(); 
	//Sets all values to false
	void setFalse(); 
	//Tells if a paricular value is possible 
	//Arguement is value to be checked. 
	//Returns TRUE if value possible else returns FALSE
	bool possible(const int&) const; 
	//Removes a particular value from set of possible values. 
	//Arguement is the value to be removed
	void remove(const int&); 
	//checks if a unique value exists
	//returns unique value of such value exists else returns 0
	int unique() const; 
	//counts number of values possible among set of possible values
	//returns number of possible values
	int numPoss() const; 
	//returns TRUE if all values flagged FALSE
	bool conflict() const; 
}; 
PossibleValues::PossibleValues(){
	for (int i = 0;  i < DIMENSION;  i++){
		values[i] = true; 
	}
}
void PossibleValues::setFalse(){
	for (int i = 0;  i < DIMENSION;  i++){
		values[i] = false; 
	}
}
bool PossibleValues::possible(const int& val) const{
	return values[val - 1]; 
}
void PossibleValues::remove(const int& val){
	if (val > 0 && val <= DIMENSION){
		values[val - 1] = false; 
	}
}
int PossibleValues::unique() const{
	if (!numPoss() || numPoss() > 1){
		return 0;
	}
	for (int i = 0;  i < DIMENSION;  i++){
		if (values[i]){
			return i + 1;
		}
	}
}
int PossibleValues::numPoss() const{
	int num(0); //counts number of possible values
	for (int i = 0;  i < DIMENSION;  i++){
		if (values[i]){
			num++;
		}
	}
	return num; 
}
bool PossibleValues::conflict() const{
	for (int i = 0;  i < DIMENSION;  i++){
		if (values[i]){
			return false;
		}
	}
	return true; 
}

class SudokuGrid{
	PossibleValues grid[DIMENSION][DIMENSION]; //set of possible solutions
	int solution[DIMENSION][DIMENSION]; //solution to grid
	bool solvable; //true if grid is solvable
	int unsolved; //number of unsolved squares
	//attempts to assign a value to a unit. 
	//Arguements are row, column and number to be assigned
	//returns FALSE if assignment secessful else returns FALSE
	bool assign(const int&, const int&, const int&); 
	//eliminates redundant/impossible values for squares acording to given constraints.
	//returns TRUE if no eliminiation breaks constraints else returns FALSE
	bool eliminate(); 
	//propogates possible solutions when elimination fails to reduce the set of solutions any further.
	//generates a solution from set of possible solutions
	//returns TRUE if propgation yeilds a solution else returns FALSE
	bool propogate(); 
	//tests weather a propogated solution satisfies constraints
	//arguements are row and column of square which is assigned one of its possible values
	//returns TRUE if assigned value yeilds a solution else returns FALSE
	bool testSquare(const int&, const int&);  
	//checks grid for squares with unique possible values and adds them to solution
	void setUnique(); 
	//functions check_* search for values in their respective units which can only attain a single square
	// under the given constraints.
	//Arguements are row and column defaulted to 0 for recursive calls
	//returns TRUE if assignments (if any) do not break constraints else returns FALSE
	bool checkPeers(const int& = 0, const int& = 0); 
	bool checkRow(const int&); 
	bool checkColumn(const int&); 
	bool checkBox(const int&, const int&); 
	//functions update_* update possible values of peers according to the solution after sucessful 
	//assignment of a value to the solution
	//arguements are row and column of square which was assigned value
	void updatePeers(const int&, const int&); 
	void updateRow(const int&, const int&); 
	void updateColumn(const int&, const int&); 
	void updateBox(const int&, const int&); 
	//functions test_* test respective units to check if the satisfy constraints
	//arguements are defaulted to 0 for recursive calls
	//returns TRUE if all constraints satisfied else returns FALSE
	bool testRows(const int& = 0) const; 
	bool testColumns(const int& = 0) const; 
	bool testBoxes(const int& = 0, const int& = 0) const; 
public:
	//constructor. Needs 81 char string for a possibly corect grid
	//string must have numbers 1 through 9 for preset squares and '.' or 0 for empty squares
	SudokuGrid(const std::string); 
	//solves a correct grid
	//returns TRUE is solution sucessful else returns FALSE
	bool solve(); 
	//checks if solution is complete. calls test_* functions
	//returns TRUE if solution complete else returns false
	bool complete() const; 
	//prints solution of puzzle
	void print() const; 
}; 
SudokuGrid::SudokuGrid(const std::string _default): solvable(true), unsolved(81){
	if (_default.length() == 81){
		for (int i = 0;  i < DIMENSION;  i++)
			for (int j = 0;  j < DIMENSION;  j++){
				solution[i][j] = 0; 
				if (_default[(i*DIMENSION) + j] - 48 > 0){
					if (!assign(i, j, _default[(i*DIMENSION) + j] - 48)){
						solvable = false;
					}
				}
			}
	}
	else {
		solvable = false;
	} 
}
bool SudokuGrid::solve(){
	if (!solvable){
		return false;
	}
	return (eliminate() && propogate()); 
}
bool SudokuGrid::eliminate(){
	if (complete()){
		return true;
	}
	int change(unsolved); //checks if any new values added to solutiom
	if (!checkPeers()){
		return false;
	}
	setUnique(); 
	if (change - unsolved){
		return eliminate();
	}
	return true; 
}
bool SudokuGrid::propogate(){
	if (complete()){
		return true;
	}
	int row(-1), column(-1), minPoss(9); 
	for (int i = 0;  i < DIMENSION;  i++){
		if (minPoss <= 2){
			break;
		}
		for (int j = 0;  j < DIMENSION;  j++){
			if (solution[i][j] || !(minPoss > grid[i][j].numPoss())){
				continue;
			}
			minPoss = grid[i][j].numPoss(); 
			row = i; 
			column = j; 
			if (minPoss <= 2){
				break;
			}
		}
	}
	if (row < 0 || column < 0){
		return false;
	}
	return testSquare(row, column); 
}
bool SudokuGrid::testSquare(const int& row,const int& column){
	int testValues(0); //test value to be assigned to square to test if it yeilds solution
	SudokuGrid testGrid(*this); //a copy of original grid used to test possible solutions
	for (int k = DIMENSION;  k > 0;  k--){
		if (testGrid.grid[row][column].possible(k)){
			testValues = k; 
			break; 
		}
	}
	if (!testValues){
		return false;
	}
	if (!testGrid.assign(row, column, testValues) || !testGrid.solve()){
		this->grid[row][column].remove(testValues); 
		return solve(); 
	}
		*this = testGrid; 
		return true; 
}
void SudokuGrid::setUnique(){
	for (int i = 0; i < DIMENSION; i++){
		for (int j = 0; j < DIMENSION; j++){
			if (!solution[i][j] && grid[i][j].unique()){
				assign(i, j, grid[i][j].unique());
			}
		}
	}
}
bool SudokuGrid::assign(const int& row, const int& column, const int& values){
	if (!grid[row][column].possible(values) || grid[row][column].conflict()){
		return false; 
	}
	grid[row][column].setFalse(); 
	solution[row][column] = values; 
	updatePeers(row, column); 
	unsolved--; 
	return true; 
}
bool SudokuGrid::checkPeers(const int& row, const int& column){
	return (checkRow(row) && checkColumn(column) && checkBox(row, column)); 
}
bool SudokuGrid::checkRow(const int& row){
	if (row >= DIMENSION){
		return true;
	}
	int count[DIMENSION]{0}; //maintains count of number of possible values for row
	for (int i = 0;  i < DIMENSION;  i++){
		if (solution[row][i]){
			count[solution[row][i] - 1] = 8; 
			for (int k = 0;  k < DIMENSION;  k++){
				count[k]++; 
			}
		}
		else{
			for (int k = 0;  k < DIMENSION;  k++){
				if (!(grid[row][i].possible(k + 1))){
					count[k]++;
				}
			}
		}
	}
	for (int k = 0;  k < DIMENSION;  k++){
		if (count[k] == 8){
			for (int i = 0;  i < DIMENSION;  i++){
				if (grid[row][i].possible(k + 1)){
					if (!assign(row, i, k + 1)){
						return false;
					}
				}
			}
		}
	}
	return checkRow(row + 1); 
}
bool SudokuGrid::checkColumn(const int& column){
	if (column >= DIMENSION){
		return true;
	}
	int count[DIMENSION]{0}; //maintains count of number of possible values for column
	for (int j = 0;  j < DIMENSION;  j++){
		if (solution[j][column]){
			count[solution[j][column] - 1] = 8; 
			for (int k = 0;  k < DIMENSION;  k++){
				count[k]++; 
			}
		}
		else{
			for (int k = 0;  k < DIMENSION;  k++){
				if (!(grid[j][column].possible(k + 1))){
					count[k]++;
				}
			}
		}
	}
	for (int k = 0;  k < DIMENSION;  k++){
		if (count[k] == 8){
			for (int j = 0;  j < DIMENSION;  j++){
				if (grid[j][column].possible(k + 1)){
					if (!assign(j, column, k + 1)){
						return false;
					}
				}
			}
		}
	}
	return checkColumn(column + 1); 
}
bool SudokuGrid::checkBox(const int& row, const int& column){
	if (row >= DIMENSION){
		return true;
	}
	int count[DIMENSION]{0}; //maintains count of number of possible values for box(row, column)
	for (int i((row / 3) * 3);  i < ((row / 3) * 3) + 3;  i++){
		for (int j((column / 3) * 3);  j < ((column / 3) * 3) + 3;  j++){
			if (solution[i][j]){
				count[solution[i][j] - 1] = 8; 
				for (int k = 0;  k < DIMENSION;  k++){
					count[k]++; 
				}
			}
			else{
				for (int k = 0;  k < DIMENSION;  k++){
					if (!(grid[i][j].possible(k + 1))){
						count[k]++;
					}
				}
			}
		}
	}
	for (int k = 0;  k < DIMENSION;  k++){
		if (count[k] == 8){
			for (int i((row / 3) * 3);  i < ((row / 3) * 3) + 3;  i++){
				for (int j((column / 3) * 3);  j < ((column / 3) * 3) + 3;  j++){
					if (grid[i][j].possible(k + 1)){
						if (!assign(i, j, k + 1)){
							return false;
						}
					}
				}
			}
		}
	}
	return checkBox(row + 1, (column + 3) % 9); 
}
void SudokuGrid::updatePeers(const int& row, const int& column){
	updateRow(row, column); 
	updateColumn(row, column); 
	updateBox(row, column); 
}
void SudokuGrid::updateRow(const int& row, const int& column){
	for (int j = 0; j < DIMENSION; j++){
		grid[row][j].remove(solution[row][column]);
	}
}
void SudokuGrid::updateColumn(const int& row, const int& column){
	for (int i = 0; i < DIMENSION; i++){
		grid[i][column].remove(solution[row][column]);
	}
}
void SudokuGrid::updateBox(const int& row, const int& column){
	for (int i = (row / 3) * 3; i < ((row / 3) * 3) + 3; i++){
		for (int j = (column / 3) * 3; j < ((column / 3) * 3) + 3; j++){
			grid[i][j].remove(solution[row][column]);
		}
	}
}
bool SudokuGrid::complete() const{
	return(!unsolved && (testRows() && testColumns() && testBoxes())); 
}
bool SudokuGrid::testRows(const int& row) const{
	if (row >= DIMENSION){
		return true;
	}
	PossibleValues test; 
	for (int i = 0;  i < DIMENSION;  i++){
		test.remove(solution[row][i]); 
	}
	if (test.conflict()){
		return testRows(row + 1);
	}
	return false; 
}
bool SudokuGrid::testColumns(const int& column) const{
	if (column >= DIMENSION)
		return true; 
	PossibleValues test; 
	for (int j = 0;  j < DIMENSION;  j++){
		test.remove(solution[j][column]); 
	}
	if (test.conflict()){
		return testColumns(column + 1);
	}
	return false; 
}
bool SudokuGrid::testBoxes(const int& row, const int& column) const{
	if (row >= DIMENSION){
		return true;
	}
	PossibleValues test; 
	for (int i((row / 3) * 3);  i < ((row / 3) * 3) + 3;  i++){
		for (int j((column / 3) * 3);  j < ((column / 3) * 3) + 3;  j++){
			test.remove(solution[i][j]); 
		}
	}
	if (test.conflict()){
		return testBoxes(row + 1, (column + 3) % 9);
	}
	return false; 
}
void SudokuGrid::print() const{
	if (!solvable){
		std::cout << "INNCORRECT PUZZLE!" << std::endl; 
		return; 
	}
	for (int i = 0;  i < DIMENSION;  i++){
		if (!(i % 3)){
			std::cout << "____________" << std::endl;
		}
		for (int j = 0;  j < DIMENSION;  j++){
			if (!(j % 3)){
				std::cout << "|";
			}
			std::cout << solution[i][j]; 
		}
		std::cout << std::endl; 
	}
}
int main(){
	std::ifstream file("list of puzzles.txt"); 
	int count(0); 
	int goodCount(0); 
	std::string sth, line; 
	while (!file.eof()){
		sth = ""; 
		for (int i = 0;  i < DIMENSION;  i++){
			std::getline(file, line); 
			if (line.length() == 81){
				sth = line; 
				break; 
			}
			if (line.length() < DIMENSION){
				i--; 
				continue; 
			}
			sth += line; 
		}
		SudokuGrid* puzzle = new SudokuGrid(sth); 
		std::cout << "PUZZLE: " << std::endl; 
		puzzle->print(); 
		puzzle->solve(); 
		count++; 
		std::cout << "SOLUTION: " << std::endl; 
		puzzle->print(); 
		std::cout << std::endl; 
		if (puzzle->complete()){
			goodCount++; 
		}
		delete puzzle; 

	}
	std::cout << "puzzles attempted: " << count << std::endl; 
	std::cout << "puzzles solved correctly out of " << count << ": " << goodCount << std::endl; 
	system("PAUSE"); 
}