//Project 2 2-bit_Histroy_Predictor
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <cstring>
#include <vector>
#include <bitset>
#include <map>

using namespace std;

struct instruction {
	int rd;
	int rs1;
	int rs2;
	int imm12;
};

struct predictor {
	int currentState;
	int state[4] = { 0,0,0,0 };
	int miss = 0;
};

int Reg[32] = {};
string State[4] = { "SN","WN","WT","ST" };

void type_I(string operation, string data, instruction& code);
int type_SB(string operation, string data, instruction& code);
void type_R(string operation, string data, instruction& code);

map< string, int > Label;
vector< string > ISA; // �h����label�Maddress��instruction

void loadTest()
{
	fstream test("test.txt", ios::in);
	for (int i = 0; test.peek() != EOF; i++)
	{
		string input, label, nonAddr;
		stringstream ss, su;
		getline(test, input);
		ss << input;
		ss >> label;
		if (label[label.size() - 1] == ':')
		{
			label.pop_back();
			Label.insert(pair<string, int>(label, i--));
		}
		else if (label[0] == '0') // �Y��0x...���F���
		{
			getline(ss, nonAddr, ';'); // Ū�촫���';'(����)
			ISA.push_back(nonAddr);
		}
		else
			ISA.push_back(input);
	}
}

void taken(predictor& pred)
{
	if (pred.state[pred.currentState] < 3)
		pred.state[pred.currentState]++;
	if (pred.currentState < 3)
		pred.currentState = (pred.currentState << 1) % 4 + 1; // �令�e�@��branch���G��ثebranch���G
}

void notTaken(predictor& pred)
{
	if (pred.state[pred.currentState] > 0)
		pred.state[pred.currentState]--;
	if (pred.currentState > 0)
		pred.currentState = (pred.currentState << 1) % 4; // �令�e�@��branch���G��ثebranch���G
}

void printEntries(predictor pred)
{
	cout << "(" << bitset<2>(pred.currentState) << ", " << State[pred.state[0]] << ", " << State[pred.state[1]] << ", " << State[pred.state[2]] << ", " << State[pred.state[3]] << ") ";
}

int main() {
	string data;
	string operation;
	instruction code;

	loadTest();

	int entry;
	cout << "Please input entry(entry > 0):" << endl;
	cin >> entry;
	vector< predictor > pred(entry);

	for (int i = 0; i < ISA.size(); i++)
	{
		//����opcode�P�_�ݩ���@��type
		stringstream ss;
		ss << ISA[i];
		ss >> operation;
		ss >> data;

		int nextPC = 0;

		//��������opcode�M�w�䬰����type
		if (operation == "beq" || operation == "bne" || operation == "blt" ||
			operation == "bge" || operation == "bltu" || operation == "bgeu")
		{
			nextPC = type_SB(operation, data, code);
			if (nextPC != -1) // �u�굲�G�� taken
			{
				if (pred[i % entry].state[pred[i % entry].currentState] < 2) // history predictor�ҹw�������G(not taken)
				{
					pred[i % entry].miss++; // misprediction
					cout << "entry: " << i % entry << ISA[i] << endl; // ��ܥثe�ϥΪ�entry
					printEntries(pred[i % entry]); // ��ܥثepredictor���A
					cout << "N" << " T" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //��ܹw�����~���ֿn����
				}
				else // history predictor�ҹw�������G(taken)
				{
					cout << "entry: " << i % entry << ISA[i] << endl; // ��ܥثe�ϥΪ�entry
					printEntries(pred[i % entry]); // ��ܥثepredictor���A
					cout << "T" << " T" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //��ܹw�����~���ֿn����
				}

				taken(pred[i % entry]); // ���branch��predictor
				i = nextPC - 1;
			}
			else // �u�굲�G�� not taken
			{
				if (pred[i % entry].state[pred[i % entry].currentState] < 2) // history predictor�ҹw�������G(not taken)
				{
					cout << "entry: " << i % entry << ISA[i] << endl; // ��ܥثe�ϥΪ�entry
					printEntries(pred[i % entry]); // ��ܥثepredictor���A
					cout << "N" << " N" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //��ܹw�����~���ֿn����
				}
				else // history predictor�ҹw�������G(taken)
				{
					pred[i % entry].miss++; // misprediction
					cout << "entry: " << i % entry << ISA[i] << endl; // ��ܥثe�ϥΪ�entry
					printEntries(pred[i % entry]); // ��ܥثepredictor���A
					cout << "T" << " N" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //��ܹw�����~���ֿn����
				}

				notTaken(pred[i % entry]);
			}

			cout << "all entries: " << endl;
			for (int j = 0; j < pred.size(); j++) // �L�X�Ҧ�entry
			{
				cout << "entry: " << j << " ";
				printEntries(pred[j]);
				cout << endl;
			}
			cout << endl;
		}

		else if (operation == "li")
		{
			stringstream su;
			string temp1, temp2;
			su << data;
			getline(su, temp1, ',');
			getline(su, temp2);
			su.str("");
			su.clear();

			temp1 = temp1.substr(1, temp1.size() - 1);
			code.rd = stoi(temp1);
			code.imm12 = stoi(temp2);

			Reg[code.rd] = code.imm12;
		}

		else if (operation == "addi" || operation == "slti" || operation == "sltiu" ||
			operation == "xori" || operation == "ori" || operation == "andi" ||
			operation == "slli" || operation == "srli" || operation == "srai")
			type_I(operation, data, code);

		else if (operation == "add" || operation == "sub" || operation == "sll" || operation == "slt" ||
			operation == "sltu" || operation == "xor" || operation == "srl" || operation == "sra" ||
			operation == "or" || operation == "and")
			type_R(operation, data, code);
	}

}

void type_I(string operation, string data, instruction& code)
{
	stringstream su;
	string temp1, temp2, temp3;
	su << data;
	getline(su, temp1, ',');
	getline(su, temp2, ',');
	getline(su, temp3);
	su.str("");
	su.clear();

	temp1 = temp1.substr(1, temp1.size() - 1);
	temp2 = temp2.substr(1, temp2.size() - 1);
	code.rd = stoi(temp1);
	code.rs1 = stoi(temp2);
	code.imm12 = stoi(temp3);

	if (operation == "addi")
		Reg[code.rd] = Reg[code.rs1] + code.imm12;
	else if (operation == "slti")
		Reg[code.rd] = (Reg[code.rs1] < code.imm12) ? 1 : 0;
	else if (operation == "sltiu")
		Reg[code.rd] = (unsigned(Reg[code.rs1]) < unsigned(code.imm12)) ? 1 : 0;
	else if (operation == "xori")
		Reg[code.rd] = Reg[code.rs1] ^ code.imm12;
	else if (operation == "ori")
		Reg[code.rd] = Reg[code.rs1] | code.imm12;
	else if (operation == "andi")
		Reg[code.rd] = Reg[code.rs1] & code.imm12;
	else if (operation == "slli")
		Reg[code.rd] = Reg[code.rs1] << code.imm12;
	else if (operation == "srli")
		Reg[code.rd] = Reg[code.rs1] >> code.imm12;
}

int type_SB(string operation, string data, instruction& code)
{
	stringstream su;
	string temp1, temp2, temp3;
	su << data;
	getline(su, temp1, ',');
	getline(su, temp2, ',');
	getline(su, temp3);
	su.str("");
	su.clear();

	temp1 = temp1.substr(1, temp1.size() - 1);
	temp2 = temp2.substr(1, temp2.size() - 1);
	code.rs1 = stoi(temp1);
	code.rs2 = stoi(temp2);

	map< string, int >::iterator it = Label.find(temp3);

	if (operation == "beq" && Reg[code.rs1] == Reg[code.rs2])
		return it->second;
	else if (operation == "bne" && Reg[code.rs1] != Reg[code.rs2])
		return it->second;
	else if (operation == "blt" && Reg[code.rs1] < Reg[code.rs2])
		return it->second;
	else if (operation == "bge" && Reg[code.rs1] >= Reg[code.rs2])
		return it->second;
	else if (operation == "bltu" && unsigned(Reg[code.rs1]) < unsigned(Reg[code.rs2]))
		return it->second;
	else if (operation == "bgeu" && unsigned(Reg[code.rs1]) >= unsigned(Reg[code.rs2]))
		return it->second;
	else
		return -1;
}

void type_R(string operation, string data, instruction& code)
{
	stringstream su;
	string temp1, temp2, temp3;
	su << data;
	getline(su, temp1, ',');
	getline(su, temp2, ',');
	getline(su, temp3);
	su.str("");
	su.clear();

	temp1 = temp1.substr(1, temp1.size() - 1);
	temp2 = temp2.substr(1, temp2.size() - 1);
	temp3 = temp3.substr(1, temp3.size() - 1);
	code.rd = stoi(temp1);
	code.rs1 = stoi(temp2);
	code.rs2 = stoi(temp3);

	if (operation == "add")
		Reg[code.rd] = Reg[code.rs1] + Reg[code.rs2];
	else if (operation == "sub")
		Reg[code.rd] = Reg[code.rs1] - Reg[code.rs2];
	else if (operation == "sll")
		Reg[code.rd] = Reg[code.rs1] << Reg[code.rs2];
	else if (operation == "slt")
		Reg[code.rd] = (Reg[code.rs1] < Reg[code.rs2]) ? 1 : 0;
	else if (operation == "sltu")
		Reg[code.rd] = (unsigned(Reg[code.rs1]) < unsigned(Reg[code.rs2])) ? 1 : 0;
	else if (operation == "xor")
		Reg[code.rd] = Reg[code.rs1] ^ Reg[code.rs2];
	else if (operation == "srl")
		Reg[code.rd] = Reg[code.rs1] >> Reg[code.rs2];
	else if (operation == "or")
		Reg[code.rd] = Reg[code.rs1] | Reg[code.rs2];
	else if (operation == "and")
		Reg[code.rd] = Reg[code.rs1] & Reg[code.rs2];
}