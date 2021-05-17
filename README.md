# ALCO_Project2
2-bit_Histroy_Predictor
## 專案說明
給定一個RISC-V的assembly code，並將所有branch instruction做prediction  

Input：一段RISC-V的assembly code，使用者可指定多少個entries  

Output：entry、目前做預測的branch instruction、predictor目前state和所有狀態、預測結果、實際結果、misprediction累積次數，以及列出所有entry之狀態  
## 程式流程
1. 讀RISC-V assembly code(.txt檔)進來  

2. 實際執行RISC-V code的程式  

3. 每遇到branch instruction就做prediction  

4. 根據每次的預測結果去修改該entry之predictor的狀態  

5. 每預測一次就輸出一次，並show出所有entry之狀態  

6. 直到RISC-V code執行結束  

## 程式碼解釋
`struct instruction` 包含 **rd** 、 **rs1** 、 **rs2** 和 **immediate**  

`struct predictor` 包含 **目前狀態** 、 **四個狀態** 和 **misprediction**  

`int Reg[32] = {}` 給定register數量並將裡面的值設成**0**  

`string State[4] = { "SN","WN","WT","ST" }` 設定**4種狀態**

`map< string, int > Label` 紀錄**label位置**  

`vector< string > ISA` 去除掉label和address的**instruction** 

***
  ```c++
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
          else if (label[0] == '0') // 若有0x...的東西時
          {
              getline(ss, nonAddr, ';'); // 讀到換行或';'(註解)
              ISA.push_back(nonAddr);
          }
          else
              ISA.push_back(input);
          }
  }
  ```
  將檔案讀進來並將label特別存起來放到`Label`[包含**key**(label)及**value**(address)]，剩餘的instruction存放至`ISA`內  
***
  ```c++
  void taken(predictor& pred)
  {
	  if (pred.state[pred.currentState] < 3)
		  pred.state[pred.currentState]++;
	  if (pred.currentState < 3)
		  pred.currentState = (pred.currentState << 1) % 4 + 1; // 改成前一個branch結果跟目前branch結果
  }

  void notTaken(predictor& pred)
  {
      if (pred.state[pred.currentState] > 0)
          pred.state[pred.currentState]--;
      if (pred.currentState > 0)
          pred.currentState = (pred.currentState << 1) % 4; // 改成前一個branch結果跟目前branch結果
  }
  ```
  根據前兩個branch的結果，去更改目前的狀態  
***
  ```c++
  void printEntries(predictor pred)
  {
	  cout << "(" << bitset<2>(pred.currentState) << ", " << State[pred.state[0]] << ", " << State[pred.state[1]] << ", " 
      << State[pred.state[2]] << ", " << State[pred.state[3]] << ") ";
  }
  ```
  輸出該entry之predictor狀態
***
  ```c++
  if (nextPC != -1) // 真實結果為 taken
  {
	  if (pred[i % entry].state[pred[i % entry].currentState] < 2) // history predictor所預測的結果(not taken)
	  {
		  pred[i % entry].miss++; // misprediction
		  cout << "entry: " << i % entry << ISA[i] << endl; // 顯示目前使用的entry
		  printEntries(pred[i % entry]); // 顯示目前predictor狀態
		  cout << "N" << " T" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //顯示預測錯誤的累積次數
	  }
	  else // history predictor所預測的結果(taken)
	  {
		  cout << "entry: " << i % entry << ISA[i] << endl; // 顯示目前使用的entry
		  printEntries(pred[i % entry]); // 顯示目前predictor狀態
		  cout << "T" << " T" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //顯示預測錯誤的累積次數
	  }

	  taken(pred[i % entry]); // 改該branch的predictor
	  i = nextPC - 1;
  }
  else // 真實結果為 not taken
  {
      if (pred[i % entry].state[pred[i % entry].currentState] < 2) // history predictor所預測的結果(not taken)
      {
          cout << "entry: " << i % entry << ISA[i] << endl; // 顯示目前使用的entry
          printEntries(pred[i % entry]); // 顯示目前predictor狀態
          cout << "N" << " N" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //顯示預測錯誤的累積次數
      }
      else // history predictor所預測的結果(taken)
      {
          pred[i % entry].miss++; // misprediction
          cout << "entry: " << i % entry << ISA[i] << endl; // 顯示目前使用的entry
          printEntries(pred[i % entry]); // 顯示目前predictor狀態
          cout << "T" << " N" << setw(18) << "misprediction: " << pred[i % entry].miss << endl; //顯示預測錯誤的累積次數
      }

      notTaken(pred[i % entry]);
  }
  ```
  顯示該branch instruction的entry預測情形，以及累積的misprediction次數
***
```c++
if (operation == "beq" || operation == "bne" || operation == "blt" ||
    operation == "bge" || operation == "bltu" || operation == "bgeu")
{
    nextPC = type_SB(operation, data, code);
    ...(顯示該branch instruction的entry預測情形，以及累積的misprediction次數)
}

else if (operation == "li")
{
    ...(切割instruction字串)

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
```
給對應的opcode決定其為哪個type
***
```c++
void type_I(string operation, string data, instruction& code)
{
    ...(切割instruction字串)

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
```
```c++
void type_R(string operation, string data, instruction& code)
{
    ...(切割instruction字串)

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
```
```c++
int type_SB(string operation, string data, instruction& code)
{
    ...(切割instruction字串)

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
```
根據該instruction的指令給予正確的運算或指示

## Sample Input
    0x110		li R2,0			; v=0 //addi R2,R0,0
    0x114		li R3,16		; Loop bound for LoopI //addi R3,R0,16
    0x118		li R4,0			; i=0 //addi R4,R0,0
            LoopI:
    0x11C		beq R4,R3,EndLoopI	; Exit LoopI if i==16  
    0x120		li R5,0			; j=0 //addi R5,R0,0  
            LoopJ:  
    0x124		beq R5,R3,EndLoopJ      ; Exit LoopJ if j==16  
    0x128		add R6,R5,R4		; j+i  
    0x12C		andi R6,R6,3		; (j+i)%4  
    0x130		bne R6,R0,Endif	        ; Skip if (j+i)%4!=0  
    0x134		add R2,R2,R5		; v+=j  
            Endif:  
    0x138		addi R5,R5,1		; j++  
    0x13C		beq R0,R0,LoopJ	        ; Go back to LoopJ  
            EndLoopJ:  
    0x140		addi R4,R4,1		; i++  
    0x144		beq R0,R0,LoopI	        ; Go back to LoopI  
            EndLoopI:  
## Sample Output
    Please input entry(entry > 0):  
    4  
    entry: 3                beq R4,R3,EndLoopI  
    (00, SN, SN, SN, SN) N N   misprediction: 0  
    all entries:  
    entry: 0 (00, SN, SN, SN, SN)  
    entry: 1 (00, SN, SN, SN, SN)  
    entry: 2 (00, SN, SN, SN, SN)  
    entry: 3 (00, SN, SN, SN, SN)  

    entry: 1                beq R5,R3,EndLoopJ  
    (00, SN, SN, SN, SN) N N   misprediction: 0  
    all entries:  
    entry: 0 (00, SN, SN, SN, SN)  
    entry: 1 (00, SN, SN, SN, SN)  
    entry: 2 (00, SN, SN, SN, SN)  
    entry: 3 (00, SN, SN, SN, SN)  

    entry: 0                bne R6,R0,Endif  
    (00, SN, SN, SN, SN) N N   misprediction: 0  
    all entries:  
    entry: 0 (00, SN, SN, SN, SN)  
    entry: 1 (00, SN, SN, SN, SN)  
    entry: 2 (00, SN, SN, SN, SN)  
    entry: 3 (00, SN, SN, SN, SN)  

    entry: 3                beq R0,R0,LoopJ  
    (00, SN, SN, SN, SN) N T   misprediction: 1  
    all entries:  
    entry: 0 (00, SN, SN, SN, SN)  
    entry: 1 (00, SN, SN, SN, SN)  
    entry: 2 (00, SN, SN, SN, SN)  
    entry: 3 (01, WN, SN, SN, SN)  

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (00, SN, SN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, SN, SN, SN)

    entry: 0                bne R6,R0,Endif
    (00, SN, SN, SN, SN) N T   misprediction: 1
    all entries:
    entry: 0 (01, WN, SN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, SN, SN, SN)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, SN, SN, SN) N T   misprediction: 2
    all entries:
    entry: 0 (01, WN, SN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, SN)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (01, WN, SN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, SN)

    entry: 0                bne R6,R0,Endif
    (01, WN, SN, SN, SN) N T   misprediction: 2
    all entries:
    entry: 0 (11, WN, WN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, SN)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, SN) N T   misprediction: 3
    all entries:
    entry: 0 (11, WN, WN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, WN)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (11, WN, WN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, WN)

    entry: 0                bne R6,R0,Endif
    (11, WN, WN, SN, SN) N T   misprediction: 3
    all entries:
    entry: 0 (11, WN, WN, SN, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, WN)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, WN) N T   misprediction: 4
    all entries:
    entry: 0 (11, WN, WN, SN, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (11, WN, WN, SN, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, WN, SN, WN) N N   misprediction: 3
    all entries:
    entry: 0 (10, WN, WN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, WT) T T   misprediction: 4
    all entries:
    entry: 0 (10, WN, WN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (10, WN, WN, SN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, WN, SN, SN) N T   misprediction: 4
    all entries:
    entry: 0 (01, WN, WN, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (01, WN, WN, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (01, WN, WN, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, WN, WN, SN) N T   misprediction: 5
    all entries:
    entry: 0 (11, WN, WT, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (11, WN, WT, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (11, WN, WT, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, WT, WN, SN) N T   misprediction: 6
    all entries:
    entry: 0 (11, WN, WT, WN, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (11, WN, WT, WN, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (11, WN, WT, WN, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, WT, WN, WN) N N   misprediction: 6
    all entries:
    entry: 0 (10, WN, WT, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (10, WN, WT, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (10, WN, WT, WN, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, WT, WN, SN) N T   misprediction: 7
    all entries:
    entry: 0 (01, WN, WT, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (01, WN, WT, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (01, WN, WT, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, WT, WT, SN) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (11, WN, ST, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, WT, SN) N T   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, WT, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, WT, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (11, WN, ST, WT, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, WT, WN) N N   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (10, WN, ST, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (10, WN, ST, WT, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, WT, SN) T T   misprediction: 8
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N N   misprediction: 0
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WN, SN, ST) T T   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, SN, SN, SN) N T   misprediction: 1
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (01, WN, SN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, SN, SN, SN) N T   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WN, SN, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, WN, SN, ST) T N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, WN, SN, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, WN, SN, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, WN, SN, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, WN, SN, WT) N T   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, WN, WN, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, WN, WN, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, WN, WN, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, WN, WN, WT) N T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, WT) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N N   misprediction: 2
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, WT, WN, ST) T T   misprediction: 7
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WN, SN, SN) N T   misprediction: 3
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (01, WN, WN, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, WN, SN, SN) N T   misprediction: 4
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, WT, WN, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, WT, WN, ST) T N   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, WT, WN, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (10, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, WT, WN, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, WT, WN, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, WT, WN, WT) N T   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, WT, WT, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, WT, WT, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, WT, WT, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, WT, WT, WT) T T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, WT) T T   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 19
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 19
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 21
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N N   misprediction: 4
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 21
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, WT, ST) T T   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, WT, SN, SN) N T   misprediction: 5
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (01, WN, WT, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, WT, SN, SN) T T   misprediction: 5
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, WT, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, WT, ST) T N   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, WT, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, WT, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, WT, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, WT, WT) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 21
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 21
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 22
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 22
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 23
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 23
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 23
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 23
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 24
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 24
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 24
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 5
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 24
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N N   misprediction: 24
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 24
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 24
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 25
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 25
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 25
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 25
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 26
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 26
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 26
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 26
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 27
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 27
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 27
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 27
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 6
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 28
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 29
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 29
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T T   misprediction: 29
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 30
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 30
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 30
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 30
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 31
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 31
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 31
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 31
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 32
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 32
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 32
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 32
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 7
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 33
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 33
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 33
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 34
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 34
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 34
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 35
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 36
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 36
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 36
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 37
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 38
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 38
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 38
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 39
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 40
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 8
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 40
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 40
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 40
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 40
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 40
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 41
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 41
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 41
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 41
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 42
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 42
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 42
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 42
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 43
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 43
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 43
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 9
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 43
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N N   misprediction: 43
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 43
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 43
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 44
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 44
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 44
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 44
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 45
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 45
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 45
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 45
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 46
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 46
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 46
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 46
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 10
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 47
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 48
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 48
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T T   misprediction: 48
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 49
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 49
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 49
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 49
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 50
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 50
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 50
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 50
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 51
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 51
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 51
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 51
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 11
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 52
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 52
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 52
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 53
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 53
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 53
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 54
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 55
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 55
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 55
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 56
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 57
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 57
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 57
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 58
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 59
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 12
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 59
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 18
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 59
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 59
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 18
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 59
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 18
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 59
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 60
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 60
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 60
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 60
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 61
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 61
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 61
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 61
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 62
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 62
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 62
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 13
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 62
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N N   misprediction: 62
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 19
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 62
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 19
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 62
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 63
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 63
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 63
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 63
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 64
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 64
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 64
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 64
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 65
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 65
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 65
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 65
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 14
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 66
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 19
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 67
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 67
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T T   misprediction: 67
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 68
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 68
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 68
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 68
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 69
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 69
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 69
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 69
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 70
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 70
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 70
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T T   misprediction: 70
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 15
    all entries:
    entry: 0 (11, WN, ST, ST, ST)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, ST) T N   misprediction: 71
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 20
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 21
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WT) T T   misprediction: 71
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 21
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (01, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WT) T T   misprediction: 71
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 72
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 21
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 72
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 72
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 73
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 74
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 74
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 74
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 75
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 76
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 76
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 76
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N T   misprediction: 77
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (11, WN, ST, ST, WT)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WT) T N   misprediction: 78
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 16
    all entries:
    entry: 0 (10, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, WN) T T   misprediction: 78
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 21
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T N   misprediction: 22
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (11, WN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, WN) T T   misprediction: 78
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (10, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (10, WN, ST, ST, WT) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (10, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (10, WN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 78
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (01, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (01, WN, ST, ST, WT) T T   misprediction: 22
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 1                beq R5,R3,EndLoopJ
    (00, WN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 78
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, WT)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, WT) T T   misprediction: 22
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 78
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 79
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 79
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 79
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 79
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 80
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 80
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 80
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 80
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, SN) N T   misprediction: 81
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (11, WN, ST, ST, WN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (11, WN, ST, ST, WN) N N   misprediction: 81
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (10, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (10, WN, ST, ST, SN) T T   misprediction: 81
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N N   misprediction: 17
    all entries:
    entry: 0 (01, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 0                bne R6,R0,Endif
    (01, WN, ST, ST, SN) T T   misprediction: 81
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R0,R0,LoopJ
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (00, SN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R5,R3,EndLoopJ
    (00, SN, ST, SN, SN) N T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (01, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 1                beq R0,R0,LoopI
    (01, WN, ST, SN, SN) T T   misprediction: 18
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)

    entry: 3                beq R4,R3,EndLoopI
    (11, WN, ST, ST, ST) T T   misprediction: 22
    all entries:
    entry: 0 (11, WN, ST, ST, SN)
    entry: 1 (11, WN, ST, SN, SN)
    entry: 2 (00, SN, SN, SN, SN)
    entry: 3 (11, WN, ST, ST, ST)
