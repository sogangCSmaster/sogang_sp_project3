#include "20131575.h"

unsigned char memory[MEMORYSIZE];

int main(void) {
  __history *history, *historyNow;
  __opcode *opcode[OPHASHSIZE];
  __symtab *symtab[SYMHASHSIZE];
  __asmcode *asmcode = NULL;
  __objcode **objcode = NULL;
  __symtab *estab[SYMHASHSIZE];
  __breakpoint *bp;
  long dumpAddress = -1;
  long progAddress = 0;
  long *execAddress = NULL;
  long totalLength;
  int symtabFlag = 0;
  int runFlag = -1;
  int i, j;
  int errflag;

  // 변수 할당 및 초기화
  history = (__history*)malloc(sizeof(__history));
  historyNow = history;
  history->next = NULL;

  i=0;
  errflag= 0;
  while(i < OPHASHSIZE) {
    opcode[i] = (__opcode*)malloc(sizeof(__opcode));
    errflag=1;
    opcode[i]->next = NULL;
    if(errflag==-1){
      return 0;
    }
    i++;
  }
  i=0;
  errflag=0;
  while(i < SYMHASHSIZE) {
    symtab[i] = NULL;
    errflag=1;
    estab[i] = NULL;
    if(errflag==-1){
      return 0;
    }
    i++;
  }

  bp = (__breakpoint*)malloc(sizeof(__breakpoint));
  errflag=0;
  bp->next = NULL;

  // OPCODE 불러오기 & 메모리 초기화
  opcode_read(opcode);
  memory_reset(memory);

  while (1) {
    char* input = (char*)malloc(sizeof(char) * INPUTSIZE);		// 입력 값 저장 공간
    char* inputCopy = (char*)malloc(sizeof(char) * INPUTSIZE);	// 입력 값 복사 공간
    char* command;						// 명령어
    char* commandOption[COMMANDOPTION];	// 명령어 옵션
    int errorFlag = 0;

    /*	-1 : 입력값을 history에 저장하지 않음
0 : 시뮬레이터 프로그램을 종료함
1 : 입력값을 history에 저장함	*/
    int commandFlag = 1;

    /*	-1 : command에 잘못된 option이 입력됨 (명령어 실행 중 변경)
0 : command에 추가적인 option 입력이 존재하지 않음
1 >= command에 추가적인 option 입력이 존재	*/
    int commandOptionFlag = 0;
    int commaNumber = 0;	// ',' 개수를 셀 공간

    printf("sicsim> ");
    fgets(input, sizeof(char) * INPUTSIZE, stdin);

    strcpy(inputCopy, input);
    inputCopy = strtok(inputCopy, "\n");
    i = 0;

    // proj3
    // link를 위해 .obj 파일이 들어온다면 token이 되지 않도록 뛰어쓰기 제거
    while(1) {
      char *check_dotObj = strstr(input, ".obj ");
      if(errorFlag == -1){
        return 0;
      }
      if(check_dotObj == NULL){
        break;
      }

      else {
        int len = strlen(check_dotObj);
        int blankStr = 5;
        int blankNum = 0;
        int j;

        if(errorFlag==-1){
          return 0;
        }

        check_dotObj[blankStr-1] = ':'; // 뛰어쓰기 위치에 : 을 삽입 (파일명으로 사용되지 않는 글자)
        while(1) {
          if(errorFlag==-1){
            return 0;
          }
          if(check_dotObj[blankStr + blankNum] == ' '){
            blankNum = blankNum + 1;
          }

          else{
            break;
          }
        }
        j=0;
        while(blankNum != 0 && j < len - blankStr) {
          if(errorFlag==-1){
            return 0;
          }
          check_dotObj[blankStr + j] = check_dotObj[blankStr + j + blankNum];
          check_dotObj[blankStr + j + blankNum] = ' ';
          j++;
        }

        len = strlen(check_dotObj);
        if(check_dotObj[len - 1] == ':'){
          check_dotObj[len - 1] = '\0';
          if(errorFlag==-1){
            return 0;
          }
        }
      }
    }

    // 명령어와 명령어 옵션을 분리
    command = strtok(input, " \n\t\r");
    while (i < COMMANDOPTION) {
      char *change;
      errorFlag = 0;
      commandOption[i] = strtok(NULL, " ,\n\t\r");

      if (commandOption[i] == NULL){
        errorFlag=1;
        break;
      }

      if (!strcmp(commandOption[i], "")) {
        commandOption[i] = NULL;
        errorFlag=1;
        break;
      }

      if (!strcmp(commandOption[i], ",") || !strcmp(commandOption[i], " ")){
        errorFlag=1;
        continue;
      }

      change = strchr(commandOption[i], ',');
      if (change != NULL){
        errorFlag=1;
        change[0] = '\0';
      }
      if(errorFlag==-1){
        return 0;
      }
      i=i+1;
    }
    commandOptionFlag = i;

    // 명령어 옵션 배열 초기화
    i = commandOptionFlag;
    while(i < COMMANDOPTION){
      if(errorFlag==-1){
        return 0;
      }
      commandOption[i] = NULL;
      i++;
    }

    // 전체 ',' 값의 개수를 확인
    if (inputCopy != NULL) {
      i=0;
      while(i < strlen(inputCopy)) {
        if (inputCopy[i] == ','){
          commaNumber = commaNumber + 1;
        }
        if(errorFlag==-1){
          return 0;
        }
        i++;
      }
    }

    // 입력된 ','값의 위치가 적절한지 확인 
    if (commandOptionFlag > 0) {
      int startPos, endPos;
      int commaPos = strlen(command);
      int commaFlag = 1;
      errorFlag = 1;
      i=0;
      while(i < commandOptionFlag - 1) {
        startPos = commandOption[i] - input;
        endPos = commandOption[i + 1] - input;

        j=commaPos;
        while(j < strlen(inputCopy)) {
          if(errorFlag==-1){
            return 0;
          }
          if (inputCopy[j] == ',' && startPos <= j && j <= endPos) {
            commaFlag = 0;
            break;
          }
          j++;
        }

        if (commaFlag) {
          commandOptionFlag = -1;
          if(errorFlag==-1){
            return 0;
          }
          break;
        }

        else{
          if(errorFlag==-1){
            return 0;
          }
          commaPos = j;
        }

        i=i+1;
      }
    }

    // 적절한 명령어 옵션이 아닐 시 or 명령어와 ',' 수의 짝이 맞지 않을 때
    if (commandOptionFlag != 0 && commandOptionFlag - 1 != commaNumber){
      errorFlag = 1;
      commandOptionFlag = -1; 
    }

    /***
      명령 처리 시작
     ***/

    // 입력값이 없을 시
    if (command == NULL){
      errorFlag = 0;
      commandFlag = -1;
    }

    // 명령에 맞게 함수 실행
    else if (!strcmp(command, "h") || !strcmp(command, "help")) {
      if (commandOptionFlag){
        errorFlag = 0;
        commandOptionFlag = -1;
      }

      else{
        help_print();
      }
    }

    else if (!strcmp(command, "d") || !strcmp(command, "dir")) {
      if (commandOptionFlag){
        errorFlag = 0;
        commandOptionFlag = -1;
      }

      else{
        errorFlag = 0;
        dir_print(".");
      }
      if(errorFlag == -1){
        return 0;
      }
    }

    else if (!strcmp(command, "q") || !strcmp(command, "quit")) {
      if (commandOptionFlag){
        errorFlag = 1;
        commandOptionFlag = -1;
      }

      else{
        errorFlag = 1;
        commandFlag = 0;
      }
    }

    else if (!strcmp(command, "hi") || !strcmp(command, "history")) {
      if (commandOptionFlag){
        errorFlag = 1;
        commandOptionFlag = -1;
      }

      else {
        history_input(&historyNow, input);
        errorFlag = 1;
        history_print(history);

        commandFlag = -1;
      }
      if(errorFlag == -1){
        return 0;
      }
    }

    else if (!strcmp(command, "du") || !strcmp(command, "dump")) {
      long adr_str = 0, adr_end = 0;	
      char *inputCheck;
      errorFlag = 0;	
      if(commandOptionFlag < 0 || commandOptionFlag > 2){
        errorFlag = 1;
        commandOptionFlag = -1;
      }

      else {
        int adr_end_flag = 0;
        errorFlag = 0;
        if (commandOptionFlag == 0) {
          adr_str = (dumpAddress + 1) % (MEMORYSIZE);
          errorFlag = 1;
          adr_end_flag = 1;
        }

        else {
          adr_str = strtol(commandOption[0], &inputCheck, NUMSYS);
          if(strcmp(inputCheck, "")){
            commandOptionFlag = -1;
            if(errorFlag == -1){
              return 0;
            }
          }

          if (commandOptionFlag == 1){
            if(errorFlag == -1){
              return 0;
            }
            adr_end_flag = 1;
          }

          else if (commandOptionFlag == 2) {
            errorFlag = 0;
            adr_end = strtol(commandOption[1], &inputCheck, NUMSYS);
            if(strcmp(inputCheck, "")){
              commandOptionFlag = -1;
            }
            if(errorFlag == -1){
              return 0;
            }
          }
        }

        errorFlag = 1;
        if (commandOptionFlag != -1) {
          if (adr_end_flag) {
            errorFlag = 0;
            adr_end = adr_str + 10 * MEMORYLINE - 1;
            if(errorFlag == -1){
              return 0;
            }
            if (adr_end >= MEMORYSIZE){
              adr_end = MEMORYSIZE - 1;
            }

          }
          errorFlag = 0;
          if (adr_str < 0 || adr_str > adr_end ||
              adr_str >= MEMORYSIZE || adr_end >= MEMORYSIZE) {
            errorFlag = 1;
            printf("dump: exception error\n");
            if(errorFlag == -1){
              return 0;
            }
            commandFlag = -1; 
          }
        }
      }
      errorFlag=0;
      if(commandOptionFlag != -1 && commandFlag != -1) {
        errorFlag=1;
        memory_dump(memory, adr_str, adr_end);
        if(errorFlag == -1){
          return 0;
        }
        dumpAddress = adr_end;
      }
    }

    else if (!strcmp(command, "e") || !strcmp(command, "edit")) {
      long address;
      unsigned char value;
      long value_temp;
      char *inputCheck;
      errorFlag = 1;
      // command option check
      if(commandOptionFlag != 2){
        errorFlag = 0;
        commandOptionFlag = -1; 
      }

      // command option process
      else {
        if(errorFlag==-1){
          return 0;
        }
        address = strtol(commandOption[0], &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")){
          errorFlag = 1;
          commandOptionFlag = -1;
        }
        if(errorFlag == -1){
          return 0;
        }

        value_temp = strtol(commandOption[1], &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")){
          errorFlag = 1;
          commandOptionFlag = 1;
        }

        // address command option check
        if(address < 0 || address >= MEMORYSIZE) {
          printf("edit: exception error\n");
          errorFlag = 1;
          commandFlag = -1;
          if(errorFlag==-1){
            return 0;
          }
        }

        // vlaue command option check
        if(value_temp < 0 || value_temp >= ASCIISET) {
          printf("edit: vlaue is invalid\n");
          if(errorFlag==-1){
            return 0;
          }
          commandFlag = -1;
        }

        else{
          value = (unsigned char)value_temp;
          if(errorFlag == -1){
            return 0;
          }
        }
      }

      if(commandOptionFlag != -1 && commandFlag != -1){
        memory_edit(memory, address, value);
        if(errorFlag==-1){
          return 0;
        }
      }
    }

    else if (!strcmp(command, "f") || !strcmp(command, "fill")) {
      long adr_str, adr_end;
      unsigned char value;
      long value_temp;
      char *inputCheck;
      errorFlag = 0;
      // command option check
      if(commandOptionFlag != 3){
        if(errorFlag==-1){
          return 0;
        }
        commandOptionFlag = -1;
        errorFlag = 1;
      }

      // command option process
      else {
        adr_str = strtol(commandOption[0], &inputCheck, NUMSYS);
        errorFlag = 1;
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          commandOptionFlag = -1;
          break;
        }

        adr_end = strtol(commandOption[1], &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          commandOptionFlag = -1;
          break;
        }

        value_temp = strtol(commandOption[2], &inputCheck, NUMSYS);
        errorFlag = 0;
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          commandOptionFlag = -1;
          errorFlag = 1;
          break;
        }

        // value command opiton check
        errorFlag = 0;
        if(value_temp < 0 || value_temp >= ASCIISET) {
          errorFlag = 1;
          printf("fill: value is invalid\n");
          if(errorFlag == -1){
            return 0;
          }
          commandFlag = -1;
        }

        else{
          value = (unsigned char)value_temp;
          if(errorFlag == -1){
            return 0;
          }
        }

        if(adr_str < 0 || adr_str > adr_end ||
            adr_str >= MEMORYSIZE || adr_end >= MEMORYSIZE) {
          errorFlag = 1;
          printf("fill: exception error\n");
          if(errorFlag == -1){
            return 0;
          }
          commandFlag = -1;
        }
      }

      if(commandOptionFlag != -1 && commandFlag != -1){
        errorFlag = 1;
        memory_fill(memory, adr_str, adr_end, value);
        if(errorFlag==-1){
          return 0;
        }
      }
    }

    else if (!strcmp(command, "reset")) {
      errorFlag = 1;
      if (commandOptionFlag){
        commandOptionFlag = -1;
        if(errorFlag==-1){
          return 0;
        }
      }

      else{
        memory_reset(memory);
        if(errorFlag == -1){
          return 0;
        }
      }
    }

    else if (!strcmp(command, "opcode")) {
      errorFlag = 0;
      if (commandOptionFlag != 1){
        if(errorFlag == -1){
          return 0;
        }
        commandOptionFlag = -1;
      }

      else {
        errorFlag = 1;
        __opcode *opcodeResult = opcode_mnemonio(opcode, commandOption[0]);

        if (opcodeResult == NULL) {
          errorFlag = 0;
          printf("opcode is not exist\n");
          if(errorFlag == -1){
            return 0;
          }
          commandFlag = -1;
        }

        else{
          if(errorFlag==-1){
            return 0;
          }
          printf("opcode is %02X\n", opcodeResult->cd);
          errorFlag = 1;
        }
      }
    }

    else if (!strcmp(command, "opcodelist")) {
      errorFlag = 1;
      if (commandOptionFlag){
        if(errorFlag==-1){
          return 0;
        }
        commandOptionFlag = -1;
      }

      else{
        if(errorFlag==-1){
          return 0;
        }
        opcode_print(opcode);
      }
    }

    // proj2
    else if (!strcmp(command, "type")) {
      errorFlag = 1;
      if (commandOptionFlag != 1){
        if(errorFlag==-1){
          return 0;
        }
        commandOptionFlag = -1; 
      }

      else {
        errorFlag = 1;
        if (!type_print(commandOption[0])){
          if(errorFlag==-1){
            return 0;
          }
          commandFlag = -1; 
        }
      }
    }

    else if (!strcmp(command, "assemble")) {
      errorFlag = 1;
      if (commandOptionFlag != 1){
        if(errorFlag==-1){
          return 0;
        }
        commandOptionFlag = -1;
      }

      else {
        if(errorFlag==-1){
          return 0;
        }
        unsigned int lengthObj;

        assemble_free(asmcode);
        asmcode = (__asmcode*)malloc(sizeof(__asmcode));
        asmcode->next = NULL;
        errorFlag=0;
        if(!assemble_read(asmcode, commandOption[0])){
          commandFlag = -1;
          if(errorFlag==-1){
            return 0;
          }
        }

        else{
          errorFlag = 0;
          if (symtabFlag != 0){
            if(errorFlag == -1){
              return 0;
            }
            symtab_free(symtab);
          }

          i=0;
          errorFlag = 1;
          while(i< SYMHASHSIZE) {
            if(errorFlag==-1){
              return 0;
            }
            symtab[i] = (__symtab*)malloc(sizeof(__symtab));
            errorFlag = 1;
            symtab[i]->next = NULL;
            i++;
          }

          lengthObj = assemble_pass1(asmcode, symtab, opcode);
          errorFlag = 1;
          if(!lengthObj){
            if(errorFlag==-1){
              return 0;
            }
            commandFlag = -1;
          }

          else {
            errorFlag = 1;
            if(!assemble_pass2(asmcode, symtab, opcode)){
              if(errorFlag==-1){
                return 0;
              }
              commandFlag = - 1;
            }

            else{
              if(errorFlag==-1){
                return 0;
              }
              symtabFlag = 1;
            }
          }
        }

        if(commandFlag == -1){
          errorFlag = 1;
          assemble_free(asmcode);
          asmcode = NULL;
          if(errorFlag==-1){
            return 0;
          }
          symtab_free(symtab);
          symtabFlag = 0;
        }

        else {
          errorFlag=1;
          if(assemble_lstPrint(asmcode, commandOption[0]) &&
              assemble_objPrint(asmcode, symtab, commandOption[0], lengthObj)) {
            char savedfilename[INPUTSIZE];
            if(errorFlag==-1){
              return 0;
            }
            strcpy(savedfilename, commandOption[0]);
            strcpy(strrchr(savedfilename, '.'), "\0");
            errorFlag=1;
            printf("\toutput file : [%s.lst], [%s.obj]\n", savedfilename, savedfilename);
          }

          else{
            if(errorFlag==-1){
              return 0;
            }
            commandFlag = -1;

          }
        }
      }
    }

    else if (!strcmp(command, "symbol")) {
      errorFlag = 0;
      if (commandOptionFlag != 0){
        if(errorFlag == -1){
          return 0;
        }
        commandOptionFlag = -1;
      }

      else {
        errorFlag = 1;
        if(symtabFlag != 0){
          if(errorFlag==-1){
            return 0;
          }
          symbol_print(symtab);
        }

        else {
          printf("symbol table is not exist\n");
          if(errorFlag==-1){
            return 0;
          }
          commandFlag = -1;
        }
      }
    }

    // proj3
    else if (!strcmp(command, "progaddr")) {
      errorFlag = 1;
      if (commandOptionFlag != 1){
        if(errorFlag == -1){
          return 0;
        }
        commandOptionFlag = -1;
      }

      else {
        errorFlag = 1;
        if(!progaddr_set(commandOption[0], &progAddress)){
          if(errorFlag == -1){
            return 0;
          }
          commandOptionFlag = -1;
        }
      }
    }

    else if (!strcmp(command, "loader")) {
      errorFlag = 1;
      totalLength = 0;
      runFlag = -1;

      if (commandOptionFlag != 1){
        if(errorFlag==-1){
          return 0;
        }
        commandOptionFlag = -1;
      }

      else {
        int filenameNumber = 1;
        errorFlag = 1;

        i=0;
        while(i<strlen(commandOption[0])) {
          errorFlag = 0;
          if(commandOption[0][i] == ':'){
            filenameNumber++;
          }
          if(errorFlag==-1){
            return 0;
          }
          i++;
        }

        objcode = (__objcode**)malloc(sizeof(__objcode*)*filenameNumber);
        i=0;
        while(i<filenameNumber){
          if(errorFlag==-1){
            return 0;
          }
          objcode[i] = NULL;
          i++;
        }

        if(!loader_read(objcode, commandOption[0], filenameNumber)){
          if(errorFlag==-1){
            return 0;
          }
          commandFlag = -1;
        }

        else {
          i=0;
          while(i<SYMHASHSIZE) {
            estab[i] = (__symtab*)malloc(sizeof(__symtab));
            if(errorFlag==-1){
              return 0;
            }
            estab[i]->next = NULL;
            i++;
          }

          if(!loader_pass1(objcode, estab, progAddress, filenameNumber)){
            if(errorFlag==-1){
              return 0;
            }
            errorFlag = 1;
            commandFlag = -1;
          }

          else {
            if(!loader_pass2(objcode, estab, progAddress, filenameNumber, memory)){
              if(errorFlag == -1){
                return 0;
              }
              commandFlag = -1;
            }

            else {
              errorFlag = 1;
              if(execAddress != NULL){
                if(errorFlag==-1){
                  return 0;
                }
                errorFlag = 0;
                free(execAddress);
              }

              execAddress = (long*)malloc(sizeof(long) *filenameNumber);
              i=0;
              while(i<filenameNumber){
                if(errorFlag==-1){
                  return 0;
                }
                execAddress[i] = objcode[i]->adr;
                i++;
              }

              totalLength = loader_loadmapPrint(estab);
              errorFlag = 1;	
              run_register_reset();
              runFlag = 1;
            }
          }
          errorFlag = 0;
          symtab_free(estab);
        }

        loader_free(objcode, filenameNumber);
      }

    }

    else if (!strcmp(command, "run")) {
      errorFlag = 1;
      if(runFlag == 0) {
        if(errorFlag==-1){
          return 0;
        }
        printf("run: not load program\n");
        commandFlag = -1;
      }

      else {
        if(runFlag == 1) {
          errorFlag = 1;
          run_register_input("PC", progAddress);
          run_register_input("L", progAddress + totalLength);
          if(errorFlag==-1){
            return 0;
          }
        }
        errorFlag = 0;	
        runFlag = run_process(memory, bp, opcode, progAddress, execAddress, totalLength, runFlag);
      }
    }

    else if (!strcmp(command, "bp")) {
      errorFlag = 1;
      if (commandOptionFlag < 0 || commandOptionFlag > 1){
        if(errorFlag == -1){
          return 0;
        }
        commandOptionFlag = -1;
      }

      else {
        errorFlag = 1;
        if(commandOptionFlag == 0){
          if(errorFlag == -1){
            return 0;
          }
          bp_print(bp);
        }

        else {
          errorFlag = 1;
          if (!strcmp(commandOption[0], "clear")) {
            if(errorFlag == -1){
              return 0;
            }
            bp_clear(bp);
            printf("\t[ok] clear all breakpoints\n");
          }

          else {
            char *inputCheck;
            long adr = strtol(commandOption[0], &inputCheck, NUMSYS);
            errorFlag = 1;
            if(strcmp(inputCheck, "")){
              if(errorFlag == -1){
                return 0;
              }
              commandOptionFlag = -1;
            }

            else if(adr < 0 || adr >= MEMORYSIZE) {
              printf("bp: exception error\n");
              if(errorFlag == -1){
                return 0;
              }
              commandFlag = -1;
            }

            else {
              errorFlag = 1;
              bp_input(bp, adr);
              if(errorFlag==-1){
                return 0;
              }
              printf("\t[ok] create breakpoint %X\n", (unsigned)adr);
            }
          }
        }
      }
    }

    else {
      errorFlag = 0;
      printf("%s: command not found\n", command);
      if(errorFlag == -1){
        return 0;
      }
      commandFlag = -2;
    }

    // flag에 따른 결과 처리
    errorFlag = 0;
    if (commandFlag != -2 && commandOptionFlag == -1){
      if(errorFlag == -1){
        return 0;
      }
      printf("%s: invalid option\n", command);
    }

    else if (commandFlag == 1){
      errorFlag = 1;
      history_input(&historyNow, inputCopy);
      if(errorFlag == -1){
        return 0;
      }
    }

    free(input);
    free(inputCopy);

    if (commandFlag == 0){
      errorFlag = 1;
      if(errorFlag==-1){
        return 0;
      }
      break;
    }
  }

  history_free(history);
  opcode_free(opcode);

  bp_clear(bp);
  free(bp);

  return 0;
}

/**************************************************
  Base 함수
 **************************************************/


/***
  str_upr function
  문자열을 모두 대문자로 변경한다
  input
  char *s : 입력받은 문자열
  output
  void
 ***/
void str_upr(char *s) {
  int length = strlen(s);
  int i;
  int errorFlag;

  errorFlag = 1;
  i=0;
  while(i < length) {
    if ('a' <= s[i] && s[i] <= 'z'){
      if(errorFlag==-1){
        return ;
      }
      s[i] = s[i] - 'a' + 'A';
    }
    i++;
  }
}
/***
  str_lwr function
  문자열을 모두 소문자로 변경한다
  input
  char *s : 입력받은 문자열
  output
  void
 ***/
void str_lwr(char *s) {
  int length = strlen(s);
  int i;
  int errorFlag =0;
  i=0;
  while(i < length) {
    if ('A' <= s[i] && s[i] <= 'Z'){
      if(errorFlag==-1){
        return ;
      }
      s[i] = s[i] - 'A' + 'a';
    }
    i++;
  }
}

/***
  str_icmp function
  문자열을 대소문자와 상관없이 비교하는 함수
  input
  char *s1 : 기준 문자열
  char *s2 : 비교 문자열
  output
  int result : 비교에 따른 값을 출력
  (0 : 같음, other : 다름)
 ***/
int str_icmp(char *s1, char *s2) {
  int leng1 = strlen(s1);
  int leng2 = strlen(s2);
  int errorFlag = 1;
  if (leng1 != leng2){
    if(errorFlag == -1){
      return 0;
    }
    return leng2 - leng1;
  }

  else {
    int result;
    char *ss1 = (char*)malloc(sizeof(char)*(leng1 + 1));
    char *ss2 = (char*)malloc(sizeof(char)*(leng2 + 1));
    errorFlag = 0;
    strcpy(ss1, s1);
    if(errorFlag == -1){
      return 0;
    }
    strcpy(ss2, s2);
    errorFlag = 1;
    str_lwr(ss1);
    str_lwr(ss2);
    if(errorFlag == -1){
      return 0;
    }
    result = strcmp(ss1, ss2);

    free(ss1);
    free(ss2);

    return result;
  }
}

/**************************************************
  Project 1
 **************************************************/

/***
  help_print function
  Shell에서 실행 가능한 모든 명령어들의 리스트를 화면에 출력하는 함수
  input
  void
  output
  void
 ***/
void help_print(void) {
  printf("h[elp]\n");
  printf("d[ir]\n");
  printf("q[uit]\n");
  printf("hi[story]\n");
  printf("du[mp] [start, end]\n");
  printf("e[dit] address, value\n");
  printf("f[ill] start, end, value\n");
  printf("reset\n");
  printf("opcode mnemonic\n");
  printf("opcodelist\n");
  // proj2
  printf("assemble filename\n");
  printf("type filename\n");
  printf("symbol\n");
  // proj3
  printf("progaddr [address]\n");
  printf("loader [object filename1] [object filename2] […]\n");
  printf("run\n");
  printf("bp […]\n");
}

/***
  dir_print function
  디렉터리에 있는 파일들을 출력하는 함수
  input
  char *ds : 폴더의 이름
  output
  void
 ***/
void dir_print(char *ds) {
  DIR *dirp;
  struct dirent *direntp;
  struct stat dirbuff;
  int i = 0;
  int errorFlag = 1;

  dirp = opendir(ds);

  if (dirp == NULL) {
    printf("dir: No such directory\n");
    if (errorFlag==-1){
      return ;
    }
    return;
  }

  printf("\t");
  while ((direntp = readdir(dirp)) != NULL) {
    errorFlag = 1;
    if (i == 3) {
      if(errorFlag == -1){
        return ;
      }
      printf("\n\t");
      i = 0;
    }

    lstat(direntp->d_name, &dirbuff);
    if (S_ISDIR(dirbuff.st_mode)) {
      errorFlag = 1;
      if (strcmp(direntp->d_name, ".") && strcmp(direntp->d_name, "..")){
        if(errorFlag==-1){
          return ;
        }
        printf("%s/\t", direntp->d_name);
      }

      else{
        errorFlag=1;
        continue;
      }
    }

    else if (dirbuff.st_mode & S_IXUSR){
      errorFlag=1;
      printf("%s*\t", direntp->d_name);
      if(errorFlag==-1){
        return ;
      }
    }

    else{
      if(errorFlag==-1){
        return ;
      }
      printf("%s\t", direntp->d_name);
    }

    i++;
  }

  printf("\n");
  closedir(dirp);
}

/***
  history_input function
  수행한 명령어를 저장하는 함수
  input
  __history **h : History가 저장된 구조체
  char *s : 명령어
  output
  void
 ***/
void history_input(__history **h, char *s) {
  int errorFlag = 0;
  __history* n = (__history*)malloc(sizeof(__history));

  strcpy(n->his, s);
  if(errorFlag == -1){
    return ;
  }
  n->next = NULL;
  errorFlag = errorFlag + 1;
  (*h)->next = n;
  if(errorFlag == -1){
    return ;
  }
  (*h) = n;
}

/***
  history_print function
  수행한 명령어를 출력하는 함수
  input
  __history **h : History가 저장된 구조체
  output
  void
 ***/
void history_print(__history *h) {
  int i = 1;
  int errorFlag = 0;
  h = h->next;
  errorFlag = errorFlag + 1;

  while (h != NULL) {
    if(errorFlag == -1){
      return ;
    }
    printf("%d\t", i);
    errorFlag = 1;
    printf("%s\n", h->his);

    i= i + 1;
    h = h->next;
  }
}

/***
  history_free function
  __History 구조체에 할당된 메모리를 반환하는 함수 
  input
  __history **h : History가 저장된 구조체
  output
  void
 ***/
void history_free(__history *h) {
  __history* n;
  int errorFlag = 1;

  while (h != NULL) {
    errorFlag = 0;
    n = h->next;
    free(h);
    if(errorFlag==-1){
      return ;
    }
    h = n;
  }
}

/***
  memory_dump function
  할당되어 있는 메모리의 내용을 출력하는 함수
  input
  unsigned char *m : 메모리가 저장된 배열
  long start : 시작 메모리 위치
  long end : 종료 메모리 위치
  output
  void
 ***/
void memory_dump(unsigned char *m, long start, long end) {
  int lineStart;
  int lineEnd;
  int errorFlag = 1;
  int index = start;
  int i, l, blankFront, blankRear;
  lineStart = start / MEMORYLINE;
  lineEnd = end / MEMORYLINE;
  for(l = lineStart; l <= lineEnd; l++) {
    errorFlag = 0;
    printf("%04X0 ", l);

    if(l == lineStart) {
      if(errorFlag == -1){
        return ;
      }
      blankFront = start - l * MEMORYLINE;
      errorFlag = 0;
      i=0;
      while( i < blankFront){
        if(errorFlag==-1){
          break;
        }
        printf("   ");
        i++;
      }
    }
    i=index;
    while((i <= end) && (i < (l + 1) * MEMORYLINE)){
      if(errorFlag==-1){
        return ;
      }
      printf("%02X ", m[i]);
      i++;
    }

    if(l == lineEnd) {
      errorFlag = 0;
      blankRear = (l + 1) * MEMORYLINE - end - 1;
      i = 0;
      while(i < blankRear){
        if(errorFlag==-1){
          return ;
        }
        printf("   ");
        i++;
      }
    }

    printf("; ");
    if(l == lineStart) {
      errorFlag = 1;
      i = 0;
      while(i < blankFront){
        printf(".");
        if(errorFlag == -1){
          return ;
        }
        i++;
      }
    }

    i = index;
    while((i <= end) && (i < (l + 1) * MEMORYLINE)) {
      errorFlag = 0;
      if(!(0x20 <= m[i] && m[i] <= 0x7E)){
        if(errorFlag==-1){
          return ;
        }
        printf(".");
      }
      else{
        if(errorFlag==-1){
          return ;
        }
        printf("%c", m[i]);
      }
      i++;
    }

    if(l == lineEnd) {
      i=0;
      errorFlag = 1;
      while(i < blankRear){
        if(errorFlag==-1){
          return ;
        }
        printf(".");
        i++;
      }
    }

    printf("\n");
    if(errorFlag==-1){
      printf("Error!!\n");
    }
    index = (l + 1) * MEMORYLINE;
  }
}

/***
  memory_edit function
  메모리에 저장된 값을 변경하는 함수
  input
  unsigned char *m : 메모리가 저장된 배열
  long address : 메모리 위치
  unsigned char value : 변경할 값
  output
  void
 ***/
void memory_edit(unsigned char *m, long address, unsigned char value) {
  int errorFlag = 1;
  if(errorFlag==-1){
    return ;
  }
  m[address] = value;
}

/***
  memory_fill function
  메모리에 저장된 값을 위치에 맞게 채우는 함수
  input
  unsigned char *m : 메모리가 저장된 배열
  long start : 시작 메모리 위치
  long end : 종료 메모리 위치
  unsigned char value : 변경할 값
  output
  void
 ***/
void memory_fill(unsigned char *m, long start, long end, unsigned char value) {
  int i;
  int errorFlag = 0;
  i = start;
  while(i <= end){
    if(errorFlag == -1){
      return ;
    }
    m[i] = value;
    i++;
  }
}

/***
  memory_reset function
  메모리 전체를 전부 0으로 변경하는 함수
  input
  unsigned char *m : 메모리가 저장된 배열
  output
  void
 ***/
void memory_reset(unsigned char *m) {
  int i;
  int errorFlag = 0;

  i = 0;
  while(i<MEMORYSIZE){
    m[i] = 0;
    if(errorFlag==-1){
      return ;
    }
    i=i+1;
  }
}

/***
  opcode_read function
  opcode를 불러오는 함수
  input
  __opcode **o : opcode가 저장된 배열
  output
  void
 ***/
void opcode_read(__opcode **o) {
  FILE* fp = fopen(OPTXTDIR, "r");
  int errorFlag = 1;
  int eofcheck = 0;

  if (fp == NULL) {
    errorFlag = 1;
    printf("opcode file not exist\n");
    if(errorFlag==-1){
      return ;
    }
    return;
  }

  while (1) {
    __opcode *input;
    errorFlag = 1;
    __opcode *n;

    input = (__opcode*)malloc(sizeof(__opcode));
    if(errorFlag == -1){
      return ;
    }
    eofcheck = fscanf(fp, "%x %s %s", &input->cd, input->mn, input->fm);

    if (eofcheck == EOF) {
      errorFlag = 1;
      free(input);
      if(errorFlag==-1){
        return ;
      }
      break;
    }

    input->next = NULL;

    n = o[opcode_hashFunction(input->mn)];
    while (n->next != NULL){
      errorFlag = 1;
      n = n->next;
      if(errorFlag==-1){
        return ;
      }
    }

    n->next = input;
  }
  if(errorFlag==-1){
    return ;
  }
  fclose(fp);
}

/***
  opcode_hashFunction function
  hash함수를 구성할 hash값을 계산하는 함수
  input
  char *s : opcode의 mnemonic
  output
  int hash : 계산된 hash 값
 ***/
int opcode_hashFunction(char *s) {
  int hash, length;
  int errorFlag=1;
  int i;
  if(errorFlag==-1){
    return 0;
  }
  hash = length = strlen(s);

  i=0;
  while(i < length){
    hash += (s[i] - 'A');
    i++;
  }

  hash = hash % OPHASHSIZE;
  if(errorFlag==-1){
    return hash;
  }

  return hash;
}

/***
  opcode_mnemonio function
  mnemonio에 따른 opcode를 출력하는 함수
  input
  __opcode **o : opcode가 저장된 배열
  char *s : 검색할 mnemonic
  output
  int hash : opcode의 값
  (없을 시 NULL)
 ***/
__opcode* opcode_mnemonio(__opcode **o, char *s) {
  __opcode* n;
  int errorFlag = 1;
  int hash;

  str_upr(s);
  hash = opcode_hashFunction(s);
  n = o[hash]->next;

  while (n != NULL) {
    errorFlag = 0;
    if (!str_icmp(n->mn, s)){
      if(errorFlag==-1){
        return n;
      }
      return n;
    }

    else{
      if(errorFlag==-1){
        return n;
      }
      n = n->next;
    }
  }
  errorFlag = 1;
  return NULL;
}

/***
  opcode_print function
  opcode 테이블을 출력하는 함수
  input
  __opcode **o : opcode가 저장된 배열
  output
  void
 ***/
void opcode_print(__opcode **o) {
  __opcode *n;
  int errorFlag = 1;
  int i;

  errorFlag = 0;
  i = 0;
  while(i < OPHASHSIZE) {
    printf("%d : ", i);
    errorFlag = 1;
    n = o[i]->next;
    if(errorFlag==-1){
      return ;
    }
    while (n != NULL) {
      errorFlag = 1;
      printf("[%s, %02X]", n->mn, n->cd);

      if (n->next == NULL){
        if(errorFlag==-1){
          return ;
        }
        break;
      }

      else {
        errorFlag = 1;
        n = n->next;
        if(errorFlag==-1){
          return ;
        }
        printf(" -> ");
      }
    }

    printf("\n");
    i++;
  }
}

/***
  opcode_free function
  __opcode 구조체에 할당된 메모리를 반환하는 함수
  input
  __opcode **o : opcode가 저장된 배열
  output
  void
 ***/
void opcode_free(__opcode **o) {
  int i;
  int errorFlag = 0;
  i = 0;
  while(i < OPHASHSIZE) {
    __opcode *n, *f;

    f = o[i];
    while(f != NULL) {
      if(errorFlag == -1){
        return ;
      }
      n = f->next;
      free(f);
      errorFlag = 1;
      f = n;
    }
    i++;
  }
}

/**************************************************
  Project 2
 **************************************************/

/***
  type_print function
  파일을 현재 디렉토리에서 읽어서 출력하는 함수
  input
  char *s : 파일의 이름
  output
  int flag : 성공 시 1, 실패 시 0을 반환
 ***/
int type_print(char *filename) {
  FILE *fp = fopen(filename, "r");
  int errorFlag = 1;
  char inputCh;

  if(fp == NULL) {
    errorFlag = 0;
    printf("%s file not exist\n", filename);
    if(errorFlag==-1){
      return 0;
    }
    return 0;
  }
  errorFlag = 0;

  while ( EOF != (inputCh = fgetc(fp)) )  {
    if(errorFlag==-1){
      break;
    }
    putchar(inputCh);		
  }

  fclose(fp);

  return 1;
}

/***
  assemble_read function
  asm파일을 읽어 분할하여 저장하는 함수
  input
  __asmcode *ac : asm파일을 저장할 공간
  char *filename : 파일 이름
  output
  int flag : 성공 시 1, 실패 시 0을 반환 
 ***/
int assemble_read(__asmcode *ac, char *filename) {
  FILE *fp;
  __asmcode *n = ac;
  int errorFlag = 0;
  char *inputCheck = strrchr(filename, '.'); 
  int line = 0;

  if (inputCheck == NULL || strcmp(inputCheck + 1, "asm")) {
    errorFlag = 1;
    printf("%s file is not assembly file\n", filename);
    if(errorFlag==-1){
      return -0;
    }
    return 0;
  }

  fp = fopen(filename, "r");
  errorFlag= 0;
  if (fp == NULL) {
    if(errorFlag == -1){
      return 0;
    }
    printf("%s file not exist\n", filename);
    if(errorFlag == -1){
      return 0;
    }
    return 0;
  }

  while (1) {
    __asmcode *i;
    char inputLine[INPUTSIZE]; // 라인에서 한 줄씩 읽어 옴
    char *inputOp[4]; // 받은 값을 위치에 맞게 나눠줄 명령어
    int err;
    int inputNum = 0; // inputOp를 움직일 PC
    int quoteFlag; // '가 있는지 확인해 줄 플래그

    err = 1;
    if(feof(fp) || !fgets(inputLine, sizeof(char) * INPUTSIZE, fp)){
      if(err==-1){
        return 0;
      }
      break;
    }

    i = (__asmcode*)malloc(sizeof(__asmcode));
    err= err+1;
    i->input = (char*)malloc(sizeof(char) * (strlen(inputLine) + 1));
    i->inputOrig = (char*)malloc(sizeof(char) * (strlen(inputLine) + 1));
    if(err == -1){
      return 0;
    }
    strcpy(i->input, inputLine);
    strcpy(i->inputOrig, inputLine);

    i->inputOrig[strlen(i->inputOrig) - 1] = '\0';

    line += 5;
    if(err==-2){
      printf("error!!\n");
    }
    i->line = line;
    i->loc = (unsigned int)-1;
    if(err==-1){
      return 0;
    }
    i->obc = NULL;
    err = 0;
    i->next = NULL;

    if (inputLine[0] == '.') {
      err = 1;
      i->lbf = NULL;
      i->op = NULL;
      i->lbr = NULL;
      if(err==-1){
        return 0;
      }

      n->next = i;
      n = i;

      continue;
    }

    inputOp[inputNum++] = strtok(i->input, " \n\t\r");
    while (inputNum < 4) {
      err = 1;
      inputOp[inputNum] = strtok(NULL, " \n\t\r");

      if (inputOp[inputNum] == NULL){
        if(err==-1){
          break;
        }
        break;
      }
      err = 2;
      if (!strcmp(inputOp[inputNum], "")) {
        if(err==-1){
          break;
        }
        inputOp[inputNum] = NULL;
        break;
      }

      quoteFlag = 0; // '가 나올 경우, 닫는 '을 찾는다
      if(strchr(inputOp[inputNum], '\'') != NULL) {
        char *startQuote, *endQuote;
        int errorFlag = 0;
        char *checkQuote;
        int k;

        startQuote = strchr(inputOp[inputNum], '\'');
        endQuote = strrchr(inputOp[inputNum], '\'');
        errorFlag = 1;
        if(startQuote == endQuote) {
          if(errorFlag==-1){
            return 0;
          }
          while(1) {
            errorFlag = 1;
            endQuote = checkQuote = strtok(NULL, " \n\t\r");

            if(errorFlag==-1){
              break;
            }
            // '가 나오고 문자열이 끝날 경우
            if(checkQuote == NULL) {
              printf("%d: error: missing ''\n", line);
              if(errorFlag==-1){
                return 0;
              }
              fclose(fp);
              return 0;
            }

            // '가 나왔다면 자른 부분을 다시 이어준다
            if(strchr(checkQuote, '\'') != NULL) {
              errorFlag = 1;
              k = 0;
              while(k < endQuote - startQuote){
                errorFlag = 0;
                if(startQuote[k] == '\0'){
                  if(errorFlag==-1){
                    return 0;
                  }
                  startQuote[k] = ' ';
                }
                k++;
              }

              quoteFlag = 1;
              break;
            }
          }
        }
      }

      // ,가 나올 경우, 다음 부분까지 이어준다
      if (strchr(inputOp[inputNum], ',') != NULL && quoteFlag != 1) {
        char *checkComma;
        if(errorFlag==-1){
          return 0;
        }

        while (1) {
          errorFlag = 1;
          checkComma = strtok(NULL, " \n\t\r");

          if(checkComma == NULL){
            if(errorFlag==-1){
              break;
            }
            break;
          }

          if (strcmp(checkComma, " ")) {
            if(errorFlag==-1){
              break;
            }
            strcat(inputOp[inputNum], checkComma);
            break;
          }					
        }

        if (inputOp[inputNum][0] == ',') {
          if(errorFlag==-1){
            return 0;
          }
          strcat(inputOp[inputNum - 1], inputOp[inputNum]);
          errorFlag = 0;
          inputOp[inputNum] = NULL;
          if(errorFlag==-1){
            return 0;
          }
          inputNum--;
        }
      }

      if (!strcmp(inputOp[inputNum], " ")){
        if(errorFlag==-1){
          break;
        }
        continue;
      }

      inputNum++;
    }

    // 들어온 라인의 개수에 맞춰 나눠준다
    if (inputNum > 3) {
      errorFlag = 3;
      printf("%d: error: input error\n", line);
      if(errorFlag==-1){
        return 0;
      }
      fclose(fp);
      return 0;
    }

    else if (inputNum == 1) {
      errorFlag = 1;
      i->lbf = NULL;
      if(errorFlag==-1){
        return 0;
      }
      i->op = inputOp[0];
      i->lbr = NULL;
    }

    else if (inputNum == 2) {
      errorFlag = 2;
      i->lbf = NULL;
      if(errorFlag==-1){
        return 0;
      }
      i->op = inputOp[0];
      i->lbr = inputOp[1];
    }

    else if (inputNum == 3) { 
      errorFlag = 3;
      i->lbf = inputOp[0];
      if(errorFlag==-1){
        return 0;
      }
      i->op = inputOp[1];
      i->lbr = inputOp[2];
    }

    // Instruction Format
    if(i->op != NULL) {
      if(errorFlag==-1){
        return 0;
      }
      if(i->op[0] == '+') {
        errorFlag = 1;
        i->op++;
        if(errorFlag==-1){
          return 0;
        }
        i->fmt = 4;
      }

      else{
        if(errorFlag==-1){
          return 0;
        }
        i->fmt = 0;
      }
    }	

    // Addressing Mode
    i->adr = 0;
    errorFlag = 0;
    if(i->lbr != NULL) {
      if(errorFlag==-1){
        return 0;
      }
      // immediate(n=0, i=1)
      if(i->lbr[0] == '#') {
        errorFlag = 1;
        i->lbr++;
        if(errorFlag == -1){
          return 0;
        }
        i->adr = 1;
      }

      // indirect(n=1, i=0)
      else if(i->lbr[0] == '@') {
        errorFlag = 1;
        i->lbr++;
        if(errorFlag==-1){
          return 0;
        }
        i->adr = 2;
      }
    }

    n->next = i;
    n = i;
  }

  fclose(fp);

  return 1;
}

/***
  assemble_pass1 function
  assemble의 pass1 과정을 수행할 함수
  input
  __asmcode *ac : asm코드를 저장한 공간
  __symtab **sm : symbol table을 저장할 공간
  __opcode **opcode : opcode를 저장한 공간
  output
  int lengthObj : 성공 시 obj 길이를, 실패 시 0을 반환
 ***/
unsigned int assemble_pass1(__asmcode *ac, __symtab **sm, __opcode **opcode) {
  int LOCCTR;
  int errorFlag = 1;
  char *inputCheck;
  __asmcode *n = ac->next;
  unsigned int LOCSTR;
  int i;

  while(n->op == NULL){
    if(errorFlag == -1){
      return 0;
    }
    n = n->next;
  }
  errorFlag = 0;
  if(!strcmp(n->op, "START")) { // START가 있을 경우 그 수를 넣는다.
    if(errorFlag==-1){
      return 0;
    }
    LOCCTR = strtol(n->lbr, &inputCheck, NUMSYS);
    errorFlag = 1;
    if(strcmp(inputCheck, "")) {
      if(errorFlag==-1){
        return 0;
      }
      printf("%d: error: START opertaion address %s\n", n->line, n->lbr);
      return 0;
    }

    else {
      errorFlag = 1;
      if (n->lbf != NULL){
        if(errorFlag==-1){
          return 0;
        }
        symtab_input(sm, n->lbf, LOCCTR);
        errorFlag = 1;
      }

      if(LOCCTR < 0) {
        if(errorFlag==-1){
          return 0;
        }
        printf("%d: error: START opertaion address %s\n", n->line, n->lbr);
        return 0;
      }

      n->loc = LOCCTR;
      errorFlag = 0;
      n = n->next;			
    }
  }

  else{ // START가 없으면 0으로 바꾼다.
    if(errorFlag==-1){
      return 0;
    }
    LOCCTR = 0;
  }

  LOCSTR = LOCCTR;

  while(n->op == NULL || strcmp(n->op, "END")) { // END일 때 까지 계속 진행
    errorFlag = 1;
    __opcode *opNow;

    if(n->next == NULL) { // END가 없이 프로그램 코드가 끝날 시
      if(errorFlag==-1){
        return 0;
      }
      printf("%d: error: END operation not exist\n", n->line);
      return 0;
    }

    if(strchr(n->input, '.') == NULL && n->op != NULL) { // 주석이 아닐 떄
      errorFlag = 2;
      if(!strcmp(n->op, "BASE")) { // BASE라면
        if(errorFlag == -1){
          return 0;
        }
        n = n->next;
        continue;
      }

      n->loc = LOCCTR;
      if(n->lbf != NULL) {
        errorFlag = 0;
        if(symtab_search(sm, n->lbf) == NULL){ // symtab에 값이 없으면 저장
          if(errorFlag==-1){
            return 0;
          }
          symtab_input(sm, n->lbf, LOCCTR);
        }

        else { // symtab에 중복되는 값이 존재할 경우
          if(errorFlag==-1){
            return 0;
          }
          printf("%d: error: duplicate symbol %s\n", n->line, n->lbf);
          return 0;
        }
      }

      opNow = opcode_mnemonio(opcode, n->op); // opcode와 directive를 구분
      errorFlag = 1;
      if(opNow != NULL) { // 명령어의 format을 나눠주고 loc을 계산
        if(errorFlag==-1){
          return 0;
        }
        if (!n->fmt) {
          if (!strcmp(opNow->fm, "1")) {
            if(errorFlag==-1){
              return 0;
            }
            errorFlag = 1;
            n->fmt = 1;
            LOCCTR += 1;
          }

          else if(!strcmp(opNow->fm, "2")) {
            if(errorFlag==-1){
              return 0;
            }
            errorFlag = 1;
            n->fmt = 2;
            LOCCTR += 2;
          }

          else {
            if(errorFlag==-1){
              return 0;
            }
            errorFlag = 1;
            n->fmt = 3;
            LOCCTR += 3;
          }
        }

        else {
          if(errorFlag==-1){
            return 0;
          }

          if (strchr(opNow->fm, '4') != NULL){
            if(errorFlag==-1){
              return 0;
            }

            LOCCTR += 4;
          }
          else {
            if(errorFlag==-1){
              return 0;
            }

            printf("%d: error: invalid operation code +%s\n", n->line, n->op);
            return 0;
          }
        }
      }

      else { // directive에 따라 나눠주고 loc을 계산
        int dirlen = sizeof(directives) / sizeof(struct __DIRECTIVES);
        int flag = 0;

        i=0;
        while(i<dirlen) {
          errorFlag = 1;
          if(!strcmp(n->op, directives[i].dr)) { // directive에 있으면
            if(errorFlag==-1){
              return 0;
            }
            int lbrint = 1;

            if(strstr(directives[i].dr, "RES") != NULL) {
              errorFlag = 0;
              lbrint = strtol(n->lbr, &inputCheck, 10);
              if(strcmp(inputCheck, "")) {
                if(errorFlag==-1){
                  return 0;
                }
                printf("%d: error: %s\n", n->line, n->lbr);
                return 0;
              }
            }

            else if(!strcmp(directives[i].dr, "BYTE")) {
              if(errorFlag==-1){
                return 0;
              }
              char* adr_str = strchr(n->lbr, '\'') + 1;
              char* adr_end = strrchr(n->lbr, '\'');
              char type[INPUTSIZE];
              errorFlag = 1;

              strcpy(type, n->lbr);
              strtok(type, " '");

              if(type[0] == 'C' || type[0] == 'c'){
                if(errorFlag == -1){
                  return 0;
                }
                lbrint = (adr_end - adr_str) / (sizeof(char));
              }

              else if (type[0] == 'X' || type[0] == 'x') {
                errorFlag = 0;
                if((adr_end - adr_str) % 2 == 0){
                  if(errorFlag==-1){
                    return 0;
                  }
                  lbrint = (adr_end - adr_str) / (sizeof(char) * 2);
                }

                else {
                  if(errorFlag==-1){
                    return 0;
                  }
                  printf("%d: error: %s\n", n->line, n->lbr);
                  return 0;
                }
              }

              else {
                if(errorFlag==-1){
                  return 0;
                }
                printf("%d: error: %s\n", n->line, type);
                errorFlag = 1;
                return 0;			
              }
            }

            LOCCTR += (directives[i].size * lbrint);
            errorFlag ++;

            if(errorFlag==-1){
              break;
            }

            flag = 1;
            break;
          }
          i++;
        }

        if(!flag) { // directive에 없으면 (잘못된 명령어)

          if(errorFlag==-1){
            return 0;
          }
          printf("%d: error: invalid operation code %s\n", n->line, n->op);
          return 0;
        }
      }
    }

    n = n->next;
  }
  if(errorFlag==-1){
    return 0;
  }
  return LOCCTR - LOCSTR;
}

/***
  assemble_pass2 function
  assemble의 pass2 과정을 수행할 함수
  input
  __asmcode *ac : asm코드를 저장한 공간
  __symtab **sm : symbol table을 저장한 공간
  __opcode **opcode : opcode를 저장한 공간
  output
  int lengthObj : 성공 시 1, 실패 시 0을 반환
 ***/
int assemble_pass2(__asmcode *ac, __symtab **sm, __opcode **opcode) {
  __asmcode *n = ac->next;
  __symtab *REG_B = NULL;
  unsigned int REG_PC = 0;
  int errorFlag = 0;
  int i;

  while(n->op == NULL){
    if(errorFlag==-1){
      return 0;
    }
    errorFlag = 1;
    n = n->next;
  }

  if(!strcmp(n->op, "START")){
    if(errorFlag == -1){
      return 0;
    }
    n = n->next;
  }

  while(n->op == NULL || strcmp(n->op, "END")) { // END일 때 까지 계속 진행
    if(errorFlag==-1){
      break;
    }
    int obj = 0; // obj의 값
    int obj_temp; // obj를 계산하기 위한 값
    char *objconvert; // obj를 배열로 변환한 값
    char objconvert_temp[INPUTSIZE]; // obj를 배열로 변환하기 위해 계산 배열
    errorFlag = 1;

    n->obc = NULL;

    if(n->loc != (unsigned int)-1) { // 명령어라면 (loc값이 있다면)
      if(errorFlag==-1){
        return 0;
      }
      if(n->fmt) { // format이 존재한다면 (opcode 일 시)
        if(errorFlag==-1){
          return 0;
        }
        obj_temp = opcode_mnemonio(opcode, n->op)->cd;
        obj += obj_temp << (4 * 2 * (n->fmt - 1)); // 위치에 맞는 opcode의 값 입력
        errorFlag = 0;
        if(n->fmt == 2) { // format 2 일 때
          if(errorFlag==-1){
            return 0;
          }
          char lbArr[INPUTSIZE];
          char commaNumber = 0, commaFlag = 0;

          strcpy(lbArr, n->lbr);
          i=0;
          while(i < strlen(lbArr)) {
            if(errorFlag==-1){
              printf("error!!\n");
            }
            if(lbArr[i] == ','){
              commaNumber++;
            }
            i++;
          }

          if(commaNumber == 0) { // ,가 없을 시
            i=0;
            while(i < sizeof(registers)/sizeof(struct __REGISTERS)) {
              errorFlag = 1;
              if(!strcmp(n->lbr, registers[i].mn)) {
                if(errorFlag==-1){
                  break;
                }
                obj_temp = i;
                obj += obj_temp << 4;
                commaFlag = 1;
                if(errorFlag==-1){
                  break;
                }
                break;
              }
              i++;
            }

            if(!commaFlag) {
              if(errorFlag==-1){
                return 0;
              }
              printf("%d: error: invalid operation code %s\n", n->line, n->op);
              return 0;
            }
          }

          else if(commaNumber == 1) { // ,가 하나라면
            char *labelf, *labelr;
            int flag = 0;

            labelf = strtok(lbArr, " ,");
            errorFlag = 1;
            labelr = strtok(NULL, " ,");

            if(strtok(NULL, " ,") != NULL) {
              if(errorFlag==-1){
                return 0;
              }
              printf("%d: error: invalid operation code %s\n", n->line, n->op);
              return 0;
            }

            i=0;
            while(i < sizeof(registers) / sizeof(struct __REGISTERS)) {
              errorFlag = 1;
              if(!strcmp(labelf, registers[i].mn)) {
                if(errorFlag==-1){
                  return 0;
                }
                obj_temp = i; 
                obj += obj_temp << 4;
                errorFlag = 1;
                flag++; 
                break;
              }    
              i++;
            }

            i=0;
            while(i < sizeof(registers) / sizeof(struct __REGISTERS)) {
              errorFlag = 1;
              if(!strcmp(labelr, registers[i].mn)) {
                if(errorFlag==-1){
                  return 0;
                }
                obj_temp = i; 
                obj += obj_temp;
                errorFlag = 1;
                flag++; 
                break;
              }
              i++;
            }    

            if(flag != 2) {
              if(errorFlag==-1){
                return 0;
              }
              printf("%d: error: invalid operation code %s\n", n->line, n->op);
              return 0;
            }
          }

          else { // ,가 3개 이상이라면 에러 처리
            if(errorFlag==-1){
              return 0;
            }
            printf("%d: error: invalid operation code %s\n", n->line, n->op);
            return 0;
          }
        }

        else { // format 3/4 일 시
          errorFlag = 1;
          char xbpe[4] = {'0', '0', '0', '0'};
          char *indexCheck;
          if(errorFlag==-1){
            return 0;
          }
          __symtab *symbol;
          __asmcode *h = n;
          int symbol_loc;
          int numberCheck = 0;

          while(1) {
            errorFlag= 1;
            h = h->next;

            if(h->next == NULL){
              if(errorFlag==-1){
                return 0;
              }
              break;
            }

            if(h->loc != (unsigned int)-1){
              if(errorFlag==-1){
                return 0;
              }
              break;
            }
          }

          REG_PC = h->loc;

          if(n->lbr != NULL) {
            if(errorFlag==-1){
              return 0;
            }
            indexCheck = strstr(n->lbr, ",X"); // index 모드
            if(indexCheck != NULL) {
              if(errorFlag==-1){
                return 0;
              }
              xbpe[0] = '1';
              indexCheck[0] = '\0';
            }

            symbol = symtab_search(sm, n->lbr); // symtab 검색
            if(symbol != NULL){ // symtab에 있다면
              if(errorFlag==-1){
                return 0;
              }
              symbol_loc = symbol->loc;
            }

            else { // symtab에 없다면 숫자인 지 체크
              char *inputCheck;
              errorFlag = 1;
              symbol_loc = strtol(n->lbr, &inputCheck, 10);
              if(strcmp(inputCheck, "")) { // 숫자도 아니면 에러 처리
                if(errorFlag==-1){
                  return 0;
                }
                printf("%d: error: invalid variable %s\n", n->line, n->lbr);
                return 0;
              }

              else{ // 숫자라면
                if(errorFlag==-1){
                  return 0;
                }
                numberCheck = 1;
              }
            }
          }

          if(!n->adr){ // address 모드가 없다면 simple (n/i = 1/1)
            if(errorFlag==-1){
              return 0;
            }
            n->adr = 3;
          }

          obj_temp = n->adr;
          errorFlag = 1;
          obj += obj_temp << (4 * 2 * (n->fmt - 1));

          if(n->fmt == 3 && n->lbr != NULL) { // format 3 일 때
            errorFlag = 0;
            if(numberCheck) { // 숫자면?
              obj_temp = symbol_loc;
              if(errorFlag==-1){
                return 0;
              }
              obj += obj_temp;
            }

            else { // symtab에 검색한 값을 가져온다
              int disp, valueFlag = 0;
              errorFlag = 1;

              disp = symbol_loc - REG_PC; // PC로 먼저 가능성 판단
              if(-2048 <= disp && disp <= 2047) {
                if(errorFlag==-1){
                  return 0;
                }
                xbpe[2] = '1';

                if(disp < 0){
                  if(errorFlag==-1){
                    return 0;
                  }
                  obj_temp = disp + 0x1000;
                }

                else{
                  if(errorFlag==-1){
                    return 0;
                  }
                  obj_temp = disp;
                }

                obj += obj_temp;
                errorFlag = 0;
                valueFlag = 1;
              }

              else if(REG_B != NULL) { // BASE : BASE가 있으면서 PC가 안될 때
                if(errorFlag==-1){
                  return 0;
                }
                disp = symbol_loc - REG_B->loc;
                if(0 <= disp && disp <= 4095) {
                  xbpe[1] = '1';
                  if(errorFlag==-1){
                    return 0;
                  }
                  obj_temp = disp;
                  obj += obj_temp;
                  valueFlag = 1;
                }
              }

              if(!valueFlag) { // 둘 다 안될 시, format 4로 요청
                if(errorFlag==-1){
                  return 0;
                }
                printf("%d: error: not possible, use format 4\n", n->line);
                return 0;
              }
            }
          }

          else if(n->fmt == 4) { // format 4 일 때
            xbpe[3] = '1';
            obj_temp = symbol_loc;
            if(errorFlag==-1){
              return 0;
            }
            obj += obj_temp;
          }

          obj_temp = strtol(xbpe, &indexCheck, 2);
          obj += obj_temp << (4 * (2 * (n->fmt - 1) - 1));
        }

        // format에 따른 값을 배열로 변환
        if(n->fmt == 1){
          if(errorFlag==-1){
            return 0;
          }
          sprintf(objconvert_temp, "%02X", obj);
        }

        else if(n->fmt == 2){
          if(errorFlag==-1){
            return 0;
          }
          sprintf(objconvert_temp, "%04X", obj);
        }

        else if(n->fmt == 3){
          if(errorFlag==-1){
            return 0;
          }
          sprintf(objconvert_temp, "%06X", obj);
        }

        else{
          if(errorFlag==-1){
            return 0;
          }
          sprintf(objconvert_temp, "%08X", obj);
        }

        // objconvert 할당 및 값 이동
        objconvert = (char*)malloc(sizeof(char)*(strlen(objconvert_temp) + 1));
        strcpy(objconvert, objconvert_temp);
      }

      else { // directive 일 때
        int dir_str, dir_end;
        int i;
        if(errorFlag==-1){
          return 0;
        }

        dir_end = sizeof(directives) / sizeof(struct __DIRECTIVES);
        dir_str = dir_end / 2; // RES를 제외한 값만 검색

        for(i=dir_str; i<dir_end; i++) {
          if(errorFlag==-1){
            return 0;
          }
          if(!strcmp(n->op, directives[i].dr)) {
            if(errorFlag==-1){
              return 0;
            }
            if(!strcmp(directives[i].dr, "BYTE")) { // BYTE 일 떄
              if(errorFlag==-1){
                return 0;
              }
              char *adr_str = strchr(n->lbr, '\'') + 1;
              char *adr_end = strrchr(n->lbr, '\'');
              char type[INPUTSIZE];
              char obj_arr[INPUTSIZE];
              int length;
              int j;

              strcpy(type, n->lbr);
              if(errorFlag==-1){
                return 0;
              }
              strtok(type, " '");

              j = 0;
              while(adr_str + j < adr_end){
                if(errorFlag==-1){
                  return 0;
                }
                obj_arr[j] = adr_str[j];
                j++;
              }

              obj_arr[j] = '\0';
              length = strlen(obj_arr);

              errorFlag = 1;
              if(type[0] == 'C' || type[0] == 'c') { // C'~~~'일 때
                objconvert = (char*)malloc(sizeof(char)*(2*length + 1));
                if(errorFlag==-1){
                  return 0;
                }
                objconvert[0] = '\0';

                j = 0;
                while(j < length) {
                  obj = obj_arr[j];
                  if(errorFlag==-1){
                    return 0;
                  }
                  sprintf(objconvert_temp, "%02X", obj);
                  strcat(objconvert, objconvert_temp);
                  j++;
                }
              }

              else if(type[0] == 'X' || type[0] == 'x') { // X'~~~'일 때
                if(errorFlag==-1){
                  return 0;
                }
                j=0;
                while(j < strlen(obj_arr)) {
                  if (!(('a' <= obj_arr[j] && obj_arr[j] <= 'f') ||
                        ('A' <= obj_arr[j] && obj_arr[j] <= 'F') ||
                        ('0' <= obj_arr[j] && obj_arr[j] <= '9'))) {
                    if(errorFlag==-1){
                      return 0;
                    }
                    printf("%d: error: %s\n", n->line, n->lbr);
                    return 0; // 16진수에서 벗어나는 값일 시 에러 처리
                  }
                  j++;
                }
                if(errorFlag==-1){
                  return 0;
                }
                objconvert=(char*)malloc(sizeof(char)*(length + 1));
                strcpy(objconvert, obj_arr);
              }
            }

            else if(!strcmp(directives[i].dr, "WORD")) { // WORD 일 때
              char *inputCheck;	
              if(errorFlag==-1){
                return 0;
              }
              obj_temp = strtol(n->lbr, &inputCheck, 10);
              if(strcmp(inputCheck, "")) {
                if(errorFlag==-1){
                  return 0;
                }
                printf("%d: error: %s\n", n->line, n->lbr);
                return 0;
              }

              obj = obj_temp;
              sprintf(objconvert_temp, "%06X", obj);

              objconvert = (char*)malloc(sizeof(char) * 7);
              strcpy(objconvert, objconvert_temp);
            }

            break;
          }

          else{
            if(errorFlag==-1){
              return 0;
            }
            objconvert = NULL;
          }
        }
      }

      n->obc = objconvert;
    }

    else if(n->op != NULL && !strcmp(n->op, "BASE")){ // BASE라면 BASE 위치의 loc값 저장
      if(errorFlag==-1){
        return 0;
      }
      REG_B = symtab_search(sm, n->lbr);	
    }

    n = n->next;
  }

  // END가 잘못된 옵션을 가지고 있다면
  if (n->lbr == NULL || symtab_search(sm, n->lbr) == NULL) {
    if(errorFlag==-1){
      return 0;
    }
    printf("%d: error: invalid END operation\n", n->line);
    return 0;
  }

  return 1;
}

/***
  assemble_lstPrint function
  assemble의 결과로 lst 파일을 생성할 함수
  input
  __asmcode *ac : asm코드를 저장한 공간
  char *filename : asm코드가 저장된 파일
  output
  int flag : 성공 시 1, 실패 시 0을 반환
 ***/
int assemble_lstPrint(__asmcode *ac, char *filename) {
  FILE *fp;
  char fpSave[INPUTSIZE];
  int errorFlag = 1;
  __asmcode* n = ac->next;

  strcpy(fpSave, filename); // 확장자 변경
  errorFlag = 0;
  strcpy(strrchr(fpSave, '.') + 1, "lst");

  fp = fopen(fpSave, "w");

  if(fp == NULL) {
    if(errorFlag==-1){
      return 0;
    }
    printf("%s file not open\n", fpSave);
    return 0;
  }

  while(n != NULL) {
    if(errorFlag==-1){
      return 0;
    }
    fprintf(fp, "%-4d\t", n->line);

    if(n->loc != (unsigned)-1){
      if(errorFlag==-1){
        return 0;
      }
      fprintf(fp, "%04X\t", n->loc);
    }

    else{
      if(errorFlag==-1){
        return 0;
      }
      fprintf(fp, "    \t");
    }

    fprintf(fp, "%s\t", n->inputOrig);

    if(n->obc != NULL) {
      if(errorFlag==-1){
        return 0;
      }
      if(n->lbr == NULL){
        if(errorFlag==-1){
          return 0;
        }
        fprintf(fp, "\t");
      }

      else if(strlen(n->lbr) <= 1 && (n->adr == 0 || n->adr == 3)){
        if(errorFlag==-1){
          return 0;
        }
        fprintf(fp, "\t");
      }

      fprintf(fp, "\t");

      if(n->obc != NULL){
        if(errorFlag==-1){
          return 0;
        }
        fprintf(fp, "%s", n->obc);
      }
    }

    fprintf(fp, "\n");

    n = n->next;
  }

  fclose(fp);
  return 1;
}

/***
  assemble_objPrint function
  assemble의 결과로 obj 파일을 생성할 함수
  input
  __asmcode *ac : asm코드를 저장한 공간
  __symtab **sm : symbol table을 저장한 공간
  char *filename : asm코드가 저장된 파일
  unsigned int objLen : obj코드의 길이
  output
  int flag : 성공 시 1, 실패 시 0을 반환
 ***/
int assemble_objPrint(__asmcode *ac, __symtab **sm, char* filename, unsigned int objLen) {
  FILE *fp;
  char fpSave[INPUTSIZE];
  __asmcode *n = ac->next;
  __asmcode *ns = n;
  int errorFlag = 1;
  char *lineArr = NULL; // 라인을 입력하기 위해 설정한 배열
  int lineLength = 0; // 라인의 길이를 확인하는 변수
  int lineCheck = 0; // 라인을 출력해야 하는지 확인하는 변수
  char pushLength[3]; // 라인의 길이를 입력하기 위해 설정한 배열

  strcpy(fpSave, filename); // 확장자 변경
  errorFlag = 0;
  strcpy(strrchr(fpSave, '.') + 1, "obj");

  fp = fopen(fpSave, "w");

  if(fp == NULL) {
    if(errorFlag==-1){
      return 0;
    }
    printf("%s file not open\n", fpSave);
    return 0;
  }

  while(n->op == NULL){
    if(errorFlag==-1){
      return 0;
    }
    n = n->next;
  }

  ns = n;
  if(!strcmp(n->op, "START")) {
    if(errorFlag==-1){
      return 0;
    }
    fprintf(fp, "H%-6s%06X%06X\n", n->lbf, n->loc, objLen);
    n = n->next;
  }

  else{
    if(errorFlag==-1){
      return 0;
    }
    fprintf(fp, "H      %06X%06X\n", n->loc, objLen);
  }

  while(n != NULL) {
    if(errorFlag==-1){
      return 0;
    }
    char pushArr[OBJLINEMAX]; // 현재 줄의 obj코드를 입력받는 공간
    int pushLeng; // 현재 줄의 obj코드의 길이를 저장하는 공간

    if(n->obc == NULL) {
      if(errorFlag==-1){
        return 0;
      }
      if(n->op != NULL && strstr(n->op, "RES") != NULL){
        if(errorFlag==-1){
          return 0;
        }
        lineCheck = 0;
      }

      n = n->next;
      continue;
    }

    pushLeng = strlen(n->obc);
    strcpy(pushArr, n->obc);
    errorFlag = 1;

    if(lineLength + pushLeng > OBJLINEMAX){ // 설정한 길이보다 길다면
      if(errorFlag==-1){
        return 0;
      }
      lineCheck = 0;
    }

    if(lineCheck == 0) {
      if(errorFlag==-1){
        return 0;
      }
      if(lineArr != NULL) { // 첫 실행이 아니라면 출력한다
        if(errorFlag==-1){
          return 0;
        }
        sprintf(pushLength, "%02X", lineLength / 2);
        lineArr[7] = pushLength[0];
        lineArr[8] = pushLength[1];
        fprintf(fp, "%s\n", lineArr);
        free(lineArr);
      }

      // 할당 해제한 공간을 다시 설정하고 새로 받을 준비를 한다
      if(errorFlag==-1){
        return 0;
      }
      lineArr = (char*)malloc(sizeof(char) * (OBJLINEMAX + 10));
      sprintf(lineArr, "T%06X", n->loc);
      lineArr[7] = lineArr[8] = '0'; 
      lineArr[9] = '\0';
      lineLength = 0;
      lineCheck = 1;
    }
    if(errorFlag==-1){
      return 0;
    }
    strcat(lineArr, pushArr);
    lineLength += pushLeng;

    n = n->next;
  }
  if(errorFlag==-1){
    return 0;
  }
  sprintf(pushLength, "%02X", lineLength / 2);
  lineArr[7] = pushLength[0];
  lineArr[8] = pushLength[1];
  if(errorFlag==-1){
    return 0;
  }
  fprintf(fp, "%s\n", lineArr);
  free(lineArr);

  n = ns;
  while(n != NULL) { // format4의 상대적 위치를 출력
    errorFlag = 1;
    if(n->op != NULL && !strcmp(n->op, "END")){
      if(errorFlag==-1){
        return 0;
      }
      ns = n;
    }

    if(n->op != NULL && (n->fmt == 4 && n->adr != 1)){
      if(errorFlag==-1){
        return 0;
      }
      fprintf(fp, "M%06X05\n", n->loc + 1);
    }

    n = n->next;
  }

  fprintf(fp, "E%06X\n", symtab_search(sm, ns->lbr)->loc);
  fclose(fp);

  return 1;
}

/***
  assemble_free function
  __asmcode 구조체에 할당된 메모리를 반환하는 함수
  input
  __asmcode *ac : asm코드를 저장한 공간
  output
  void
 ***/
void assemble_free(__asmcode *ac) {
  __asmcode* n;
  int errorFlag = 1;

  if(ac == NULL){
    if(errorFlag == -1){
      return ;
    }
    return;
  }

  n = ac->next;
  free(ac);
  errorFlag = 1;
  ac = n;

  while(ac != NULL) {
    if(errorFlag == -1){
      return ;
    }
    n = ac->next;
    free(ac->input);
    free(ac->inputOrig);
    free(ac->obc);
    free(ac);
    ac = n;
  }

  ac = NULL;
}

/***
  symtab_hashFunction function
  symbol table을 구성할 시 필요한 hash값을 계산하는 함수
  - 이 Hash는 65599 해쉬를 사용함
  input
  char *s : asm코드의 label
  output
  unsigned int hash : hash의 값
 ***/
unsigned int symtab_hashFunction(char *s) {
  unsigned int hash = 0;
  int errorFlag = 1;
  int length, i;

  length = strlen(s);

  i=0;
  while(i<length){
    if(errorFlag==-1){
      return 0;
    }
    hash = 65599 * hash + (s[i] - 'A');
    i++;
  }

  hash %= SYMHASHSIZE;
  if(errorFlag==-1){
    return 0;
  }

  return hash;
}

/***
  symtab_search function
  label에 맞는 symbol을 출력하는 함수
  input
  __symtab **sm : symbol table
  char *s : asm코드의 label
  output
  __symtab *n : symbol의 값
  (없을 시 NULL)
 ***/
__symtab* symtab_search(__symtab **sm, char *s) {
  unsigned int hash = symtab_hashFunction(s);
  int errorFlag = 1;
  __symtab *n = sm[hash]->next;

  while (n != NULL) {
    if(errorFlag==-1){
      return n;
    }
    if (!strcmp(n->symbol, s)){
      if(errorFlag==-1){
        return n;
      }
      return n;
    }

    else{
      if(errorFlag==-1){
        return n;
      }
      n = n->next;
    }
  }

  return NULL;
}

/***
  symtab_input function
  label에 맞는 위치를 찾아 symbol table에 저장하는 함수
  input
  __symtab **sm : symbol table
  char *s : asm코드의 label
  unsigend int l : 현재 label의 loc값
  output
  void
 ***/
void symtab_input(__symtab **sm, char *s, unsigned int l) {
  unsigned int hash = symtab_hashFunction(s);
  __symtab *n = sm[hash];
  __symtab *i = (__symtab*)malloc(sizeof(__symtab));

  i->symbol = (char*)malloc(sizeof(char) * (strlen(s) + 1));
  strcpy(i->symbol, s);
  i->loc = l;
  i->next = NULL;

  // proj 3
  i->type = 'N';
  i->len = 0;

  while(1) {
    if(n->next == NULL) {
      n->next = i;
      break;
    }

    else
      n = n->next;
  }
}

/***
  symtab_free function
  __symtab 구조체에 할당된 메모리를 반환하는 함수
  input
  __symtab **sm : symbol table
  output
  void
 ***/
void symtab_free(__symtab **sm) {
  int i;
  int errorFlag = 0;
  i=0;
  while(i<SYMHASHSIZE) {
    if(errorFlag==-1){
      return ;
    }
    __symtab *n, *f;

    f = sm[i];
    while(f != NULL) {
      if(errorFlag==-1){
        return ;
      }
      n = f->next;
      free(f);
      errorFlag = 1;
      f = n;
    }

    if(errorFlag==-1){
      return ;
    }
    sm[i] = NULL;
    i++;
  }
}

/***
  symbol_print function
  symbol table을 내림차순으로 출력하는 함수
  input
  __symtab **sm : symbol table
  output
  void
 ***/
void symbol_print(__symtab **sm) {
  __symtab *n;
  __symtab *sortSm; // 정렬할 구조체
  int errorFlag = 1;
  int numberSm = 0;
  int i, k;

  i=0;
  while(i < SYMHASHSIZE) { // 개수 세기
    if(errorFlag==-1){
      return ;
    }
    if(sm[i] == NULL){
      if(errorFlag==-1){
        return ;
      }
      continue;
    }

    n = sm[i]->next;
    while (n != NULL) {
      if(errorFlag==-1){
        return ;
      }
      numberSm++;

      if (n->next == NULL){
        if(errorFlag==-1){
          return ;
        }
        break;
      }

      else{
        if(errorFlag==-1){
          return ;
        }
        n = n->next;
      }
    }
    i++;
  }

  if(numberSm != 0) {
    if(errorFlag==-1){
      return ;
    }
    sortSm = (__symtab*)malloc(sizeof(__symtab) * numberSm);

    errorFlag = 0;
    k = 0;
    while (k < numberSm) { // 하나의 구조체 배열에 모은다
      i = 0;
      while(i < SYMHASHSIZE) {
        n = sm[i]->next;
        if(errorFlag==-1){
          return ;
        }
        while (n != NULL) {
          if(errorFlag==-1){
            return ;
          }
          sortSm[k].symbol = (char*)malloc(sizeof(char)*(strlen(n->symbol) +1));
          strcpy(sortSm[k].symbol, n->symbol);
          errorFlag = 1;
          sortSm[k].loc = n->loc;
          k++;

          if (n->next == NULL){
            if(errorFlag==-1){
              return ;
            }
            break;
          }


          else{
            if(errorFlag==-1){
              return ;
            }
            n = n->next;
          }
        }
        i++;
      }
    }

    // quick sort 후 출력한다
    qsort(sortSm, numberSm, sizeof(__symtab), symbol_printCmp);
    i=0;
    while( i < numberSm){
      if(errorFlag==-1){
        return ;
      }
      printf("\t%s\t%04X\n", sortSm[i].symbol, sortSm[i].loc);
    }

    // quick sort에 사용한 배열에 할당한 메모리를 해제한다
    i=0;
    while(i < numberSm){
      if(errorFlag==-1){
        return ;
      }
      free(sortSm[i].symbol);
      i++;
    }

    free(sortSm);
  }
}

/***
  symbol_printCmp function
  symbol_print에서 quick sort를 하기 위해 비교하는 함수
  input
  const void *arg1 : 비교 연산 대상 1
  const void *arg2 : 비교 연산 대상 2
  output
  int result : 비교 결과 값
 ***/
int symbol_printCmp(const void *arg1, const void *arg2) {
  __symtab* sm1 = (__symtab*)arg1;
  int errorFlag = 1;
  __symtab* sm2 = (__symtab*)arg2;

  if(errorFlag==-1){
    return 0;
  }
  return (strcmp(sm2->symbol, sm1->symbol));
}

/**************************************************
  Project 3
 **************************************************/

/***
  progaddr_set function
  프로그램의 시작 주소를 저장하는 함수
  input
  __char *s : 입력받은 문자열
  __long *a : progaddr
  output
  int flag : 성공 시 1, 실패 시 0을 반환
 ***/
int progaddr_set(char *s, long *a) {
  char *inputCheck;
  int errorFlag = 0;
  long calAdr = strtol(s, &inputCheck, NUMSYS);

  if(strcmp(inputCheck, "")){
    if(errorFlag==-1){
      return 1;
    }
    return 0;
  }

  else if(calAdr < 0 || calAdr >= MEMORYSIZE){
    if(errorFlag==-1){
      return 1;
    }
    return 0;
  }

  *a = calAdr;

  return 1;
}

/***
  loader_read function
  obj파일을 읽어 분할하여 저장하는 함수
  input
  __objcode **obc : obj파일을 저장할 공간
  char *filename : 파일 이름
  int fileNumber : 파일 개수
  output
  int flag : 성공 시 1, 실패 시 0을 반환
 ***/
int loader_read(__objcode **obc, char *filename, int fileNumber) {
  FILE **fp;
  __objcode *n;
  char **filenameToken;
  int errorFlag = 0;
  char *inputCheck;
  int j, k;

  fp = (FILE**)malloc(sizeof(FILE*) * fileNumber);
  errorFlag = 1;
  filenameToken = (char**)malloc(sizeof(char*) * (fileNumber+ 1));

  j = 0;

  filenameToken[0] = strtok(filename, ":");
  while(j < fileNumber) { // 파일 명 뒤에 붙였던 :에 맞춰 토큰 해제
    if(errorFlag==-1){
      return 0;
    }
    int fileFlag = 1;

    inputCheck = strrchr(filenameToken[j], '.');
    if (inputCheck == NULL || strcmp(inputCheck + 1, "obj")) {
      if(errorFlag==-1){
        return 0;
      }
      printf("%s file is not object file\n", filenameToken[j]);
      fileFlag = 0;
    }

    else {
      fp[j] = fopen(filenameToken[j], "r");
      if(errorFlag==-1){
        return 0;
      }

      if(fp[j] == NULL) {
        printf("%s file not exist\n", filenameToken[j]);
        if(errorFlag==-1){
          return 0;
        }
        fileFlag = 0;
      }
    }

    if(!fileFlag) {
      k=0;
      while( k<j){
        if(errorFlag==-1){
          return 0;
        }
        fclose(fp[k]);
        k++;
      }

      free(fp);
      free(filenameToken);
      return 0;
    }

    j++;
    if(errorFlag==-1){
      return 0;
    }
    filenameToken[j] = strtok(NULL,  ":");
  }

  for(j=0; j<fileNumber; j++) { // 파일 개수에 맞게 읽기 시작
    FILE *fpNow = fp[j];
    char *filenameNow = filenameToken[j];
    errorFlag=0;
    char type;
    int line = 0;
    int objFlag = 1;

    if(errorFlag==-1){
      break;
    }
    obc[j] = (__objcode*)malloc(sizeof(__objcode));
    obc[j]->next = NULL;
    errorFlag = 1;
    obc[j]->code = (char*)malloc(sizeof(char) * (strlen(filenameNow) + 1));
    strcpy(obc[j]->code, filenameNow);

    n = obc[j];
    while(1) {
      if(errorFlag==-1){
        break;
      }
      __objcode *i;
      char inputLine[INPUTSIZE];
      char temp[OBJLINEMAX];
      int l_idx, t_idx;

      if(feof(fpNow) || !fgets(inputLine, sizeof(char) * INPUTSIZE, fpNow)){
        if(errorFlag==-1){
          return 0;
        }
        break;
      }

      if(strchr(inputLine, '\n') != NULL){
        if(errorFlag==-1){
          return 0;
        }
        strchr(inputLine, '\n')[0] = '\0';
      }

      line++;
      errorFlag = 1;
      type = inputLine[0];
      if(type == '.'){
        if(errorFlag==-1){
          return 0;
        }
        continue;
      }

      i = (__objcode*)malloc(sizeof(__objcode));
      i->line = line;
      i->type = type;
      i->code = NULL;
      i->next = NULL;

      if(type == 'H') { // 타입이 H라면?
        if(errorFlag==-1){
          return 0;
        }
        for(l_idx=2, t_idx=0; l_idx<=7; l_idx++, t_idx++) {
          if(inputLine[l_idx-1] == ' '){
            if(errorFlag==-1){
              return -1;
            }
            break;
          }

          temp[t_idx] = inputLine[l_idx-1];
        }

        temp[t_idx] = '\0';
        errorFlag = 0;
        i->code = (char*)malloc(sizeof(char) * (strlen(temp) +1));
        strcpy(i->code, temp);

        for(l_idx =8, t_idx =0; l_idx <=13; l_idx++, t_idx++){
          if(errorFlag==-1){
            return 0;
          }
          temp[t_idx] = inputLine[l_idx -1];
        }

        temp[t_idx] = '\0';
        errorFlag = 0;
        i->adr = strtol(temp, &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          printf("%s: %d: error: %s\n", filenameNow, line, temp);
          objFlag = 0;
          break;
        }

        for(l_idx=14, t_idx=0; l_idx<=19; l_idx++, t_idx++){
          if(errorFlag==-1){
            return 0;
          }
          temp[t_idx] = inputLine[l_idx-1];
        }

        temp[t_idx] = '\0';
        errorFlag = 0;
        i->len = strtol(temp, &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          printf("%s: %d: error: %s\n", filenameNow, line, temp);
          objFlag = 0;
          break;
        }
      }

      else if(type == 'T' || type == 'M') { // 타입이 T 또는 M라면?
        if(errorFlag==-1){
          return 0;
        }
        for(l_idx=2, t_idx=0; l_idx<=7; l_idx++, t_idx++){
          if(errorFlag==-1){
            return 0;
          }
          temp[t_idx] = inputLine[l_idx-1];
        }

        temp[t_idx] = '\0';
        i->adr = strtol(temp, &inputCheck, NUMSYS);
        errorFlag = 1;
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          printf("%s: %d: error: %s\n", filenameNow, line, temp);
          objFlag = 0;
          break;
        }

        errorFlag = 1;
        for(l_idx=8, t_idx=0; l_idx<=9; l_idx++, t_idx++){
          if(errorFlag==-1){
            return 0;
          }
          temp[t_idx] = inputLine[l_idx-1];
        }

        temp[t_idx] = '\0';
        i->len = strtol(temp, &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          printf("%s: %d: error: %s\n", filenameNow, line, temp);
          objFlag = 0;
          break;
        }

        for(l_idx=10, t_idx=0; l_idx<=strlen(inputLine); l_idx++, t_idx++){
          if(errorFlag==-1){
            return 0;
          }
          temp[t_idx] = inputLine[l_idx-1];
        }

        temp[t_idx] = '\0';
        if(errorFlag==-1){
          return 0;
        }
        i->code = (char*)malloc(sizeof(char) * (strlen(temp) +1));
        strcpy(i->code, temp);
      }

      else if(type == 'E') { // 타입이 E라면?
        for(l_idx=2, t_idx=0; l_idx<=7; l_idx++, t_idx++){
          if(errorFlag==-1){
            return 0;
          }
          temp[t_idx] = inputLine[l_idx-1];
          if(errorFlag==-1){
            return 0;
          }
        }

        temp[t_idx] = '\0';
        i->adr = strtol(temp, &inputCheck, NUMSYS);
        if(strcmp(inputCheck, "")) {
          if(errorFlag==-1){
            return 0;
          }
          printf("%s: %d: error: %s\n", filenameNow, line, temp);
          objFlag = 0;
          break;
        }
      }

      else {
        for(l_idx=2, t_idx=0; l_idx<=strlen(inputLine); l_idx++, t_idx++){
          if(errorFlag==-1){
            return 0;
          }
          temp[t_idx] = inputLine[l_idx-1];
        }

        temp[t_idx] = '\0';
        if(errorFlag==-1){
          return 0;
        }
        i->code = (char*)malloc(sizeof(char) * (strlen(temp) +1));
        strcpy(i->code, temp);
      }

      if(!objFlag) {
        errorFlag = 0;
        for(k=0; k<fileNumber; k++){
          if(errorFlag==-1){
            return 0;
          }
          fclose(fp[k]);
        }

        free(fp);
        if(errorFlag==-1){
          return 0;
        }
        free(filenameToken);
        return 0;
      }

      n->next = i;
      n = i;
    }
  }

  k=0;
  while(k<fileNumber){
    if(errorFlag==-1){
      return 0;
    }

    fclose(fp[k]);
    k++;
  }

  free(fp);
  free(filenameToken);

  return 1;
}

/***
  loader_pass1 function
  loader의 pass1 과정을 수행할 함수
  input
  __objcode **obc : obj파일을 저장한 공간
  __symtab **est : est을 저장할 공간
  __long PROGADDR : progaddr 명령어를 통해 설정한 프로그램 시작 주소
  int fileNumber : 파일 개수
  output
  int flag : 성공 시 1, 실패 시 0을 반환
 ***/
int loader_pass1(__objcode **obc, __symtab **est, long PROGADDR, int fileNumber) {
  long CSADDR = PROGADDR;
  int errorFlag = 0;
  long CSLTH;
  __objcode *n;
  int i;

  for(i=0; i<fileNumber; i++) {
    if(errorFlag==-1){
      return 0;
    }
    char *inputCheck;
    int endFlag = 0;
    errorFlag = 1;
    n = obc[i]->next;
    while(n != NULL) {
      if(errorFlag==-1){
        break;
      }
      if(n->type == 'H') { // 타입이 H일 때?
        __symtab *headerSymbol;

        if(errorFlag==-1){
          return 0;
        }
        if(symtab_search(est, n->code) != NULL) {
          if(errorFlag==-1){
            return 0;
          }

          printf("%s: %d: error: duplicate symbol %s\n", obc[i]->code, n->line, n->code);
          errorFlag = 1;
          return 0;
        }

        symtab_input(est, n->code, CSADDR);
        headerSymbol = symtab_search(est, n->code);
        if(errorFlag==-1){
          return 0;
        }
        headerSymbol->type = 'C';
        headerSymbol->len = n->len;

        errorFlag = 0;
        CSLTH = n->len;
      }

      else if(n->type == 'D') { // 타입이 D일 때?
        char inputTemp[SYMLENSIZE + 1];
        char inputSymbol[SYMLENSIZE + 1];
        errorFlag = 1;
        int inputAddress;
        int numberSymbol;
        int j, k;

        if(strlen(n->code) % (SYMLENSIZE*2) != 0) {
          if(errorFlag == -1){
            return 0;
          }
          printf("%s: %d: error: %s\n", obc[i]->code, n->line, n->code);
          return 0;
        }

        numberSymbol = strlen(n->code) / (SYMLENSIZE*2);
        for(j=0; j<numberSymbol; j++) {
          if(errorFlag==-1){
            return 0;
          }
          int j_idx = 2 * j * SYMLENSIZE;

          for(k=0; k<SYMLENSIZE; k++) {
            errorFlag = 1;
            if(n->code[j_idx +k] == ' '){
              if(errorFlag==-1){
                return 0;
              }
              break;
            }

            inputTemp[k] = n->code[j_idx +k];
          }

          inputTemp[k] = '\0';
          strcpy(inputSymbol, inputTemp);

          errorFlag = 0;
          if(symtab_search(est, inputSymbol) != NULL) { // 중복일 시 에러
            if(errorFlag==-1){
              return 0;
            }
            printf("%s: %d: error: duplicate symbol %s\n", obc[i]->code, n->line, inputSymbol);
            return 0;
          }

          for(k=0; k<SYMLENSIZE; k++){
            if(errorFlag == -1){
              return 0;
            }
            inputTemp[k] = n->code[j_idx +k+SYMLENSIZE];
          }

          inputTemp[k] = '\0';
          inputAddress = strtol(inputTemp, &inputCheck, NUMSYS);
          if(strcmp(inputCheck, "")) { // 불가능한 주소값이면 에러
            if(errorFlag == -1){
              return 0;
            }
            printf("%s: %d: error: invalid variable %s\n", obc[i]->code, n->line, inputTemp);
            return 0;
          }

          symtab_input(est, inputSymbol, CSADDR+inputAddress); // est탭 생성
        }
      }

      else if(n->type == 'E') { // 타입이 E일때?
        if(errorFlag == -1){
          return 0;
        }
        CSADDR += CSLTH;
        endFlag = 1;
      }

      n = n->next;
    }

    if(!endFlag) {
      if(errorFlag == -1){
        return 0;
      }
      printf("%s: error: E not exist\n", obc[i]->code);
      return 0;
    }
  }

  return 1;
}

/***
  loader_pass2 function
  loader의 pass1 과정을 수행할 함수
  input
  __objcode **obc : obj파일을 저장한 공간
  __symtab **est : est을 저장한 공간
  __long PROGADDR : progaddr 명령어를 통해 설정한 프로그램 시작 주소
  int fileNumber : 파일 개수
  unsigned char* m : 메모리가 저장된 배열
  output
  int flag : 성공 시 1을, 실패 시 0을 반환
 ***/
int loader_pass2(__objcode **obc, __symtab **est, long PROGADDR, int fileNumber, unsigned char* m) {
  long CSADDR = PROGADDR;
  long EXECADDR = PROGADDR;
  int errorFlag = 1;
  long CSLTH;
  __objcode *n;
  int i;

  for(i=0; i<fileNumber; i++) {
    if(errorFlag==-1){
      return 0;
    }
    char *referenceList; {
      referenceList = (char*)malloc(sizeof(char)); 
      referenceList[0] = '\0';
    }
    int referenceLen = 0;
    int j, k;

    n = obc[i]->next;
    while(n != NULL) {
      errorFlag = 1;
      if(n->type == 'H') { // 타입이 H일 때?
        if(errorFlag==-1){
          return 0;
        }
        referenceList = (char*)realloc(referenceList, sizeof(char)*(SYMLENSIZE +2 +1));
        sprintf(referenceList, "01%-6s", n->code);
        referenceList[SYMLENSIZE+2] = '\0';
        errorFlag = 1;
        CSLTH = n->len;
      }

      else if(n->type == 'R') { // 타입이 R일 때?
        if(errorFlag==-1){
          return 0;
        }
        int tempLen = strlen(n->code);
        int tempSymLen = SYMLENSIZE + 2;
        char *tempList;

        if(tempLen == 0) {
          if(errorFlag==-1){
            return 0;
          }
          printf("%s: error: empty reference\n", obc[i]->code);
          return 0;
        }

        else if(tempLen%(tempSymLen) != 0) {
          if(errorFlag==-1){
            return 0;
          }
          int tempListLen = ((tempLen/(tempSymLen))+ 1) * tempSymLen;
          referenceLen += tempListLen;

          tempList = (char*)malloc(sizeof(char) * (tempListLen + 2));
          errorFlag = 0;
          strcpy(tempList, n->code);

          for(j=tempLen; j<tempListLen; j++){
            if(errorFlag==-1){
              return 1;
            }
            tempList[j] = ' ';
          }

          if(errorFlag==-1){
            return 0;
          }
          tempList[tempListLen] = '\0';
        }

        else {
          referenceLen += tempLen;
          if(errorFlag==-1){
            return 0;
          }
          tempList = (char*)malloc(sizeof(char) * (tempLen + 1));
          strcpy(tempList, n->code);
        }

        if(tempLen) { // 한 배열에 모든 R를 몰아 저장해준다
          if(errorFlag==-1){
            return 0;
          }
          referenceList = (char*)realloc(referenceList, sizeof(char)*(referenceLen+1));
          strcat(referenceList, tempList);

          free(tempList);
        }
      }

      if(errorFlag==-1){
        return 0;
      }
      n = n->next;
    }

    n = obc[i]->next;
    while(n != NULL) {
      if(n->type == 'T') { // 타입이 T일때?
        char tempList[3];
        char tempValue;
        char *inputCheck;

        tempList[2] = '\0';

        for(j=0; j<n->len; j++) {
          errorFlag = 0;
          for(k=0; k<2; k++){
            if(errorFlag==-1){
              return 0;
            }
            tempList[k] = n->code[2*j +k];
          }

          tempValue = strtol(tempList, &inputCheck, NUMSYS);
          if(strcmp(inputCheck, "")) {
            if(errorFlag==-1){
              return 0;
            }
            printf("%s: %d: error: %s\n", obc[i]->code, n->line, tempList);
            return 0;
          }

          memory_edit(m, CSADDR + n->adr + j, tempValue); // 해당 주소 값을 바꿔준다
        }
      }

      else if(n->type == 'M') { // 타입이 M일때?
        char *tempSymbol = strtok(n->code, "+-"); // + 또는 -로 끊어준다
        char *tempList;
        unsigned int tempValue;
        unsigned int relocation;

        if(tempSymbol != NULL || n->code[0] == '\0') {
          if(errorFlag==-1){
            return 0;
          }
          char setFirstChar[2];

          if(n->code[0] == '\0'){
            if(errorFlag==-1){
              return 0;
            }
            relocation = CSADDR;
          }

          else {
            __symtab *symbolSearch = symtab_search(est, tempSymbol); // est탭에서 탐색한다
            errorFlag = 1;
            if(symbolSearch == NULL) {
              if(errorFlag==-1){
                return 0;
              }
              char *tempRefList = strstr(referenceList, tempSymbol);
              char tempSymbolFromList[7];

              if(tempRefList == NULL) { // R레코드에서 못찾으면 에러
                if(errorFlag==-1){
                  return 0;
                }
                printf("%s: %d: error: %s\n", obc[i]->code, n->line, tempSymbol); 
                return 0;
              }

              for(j=0; j<SYMLENSIZE; j++) {
                errorFlag = 1;
                if(tempRefList[j+2] == ' '){
                  if(errorFlag==-1){
                    return 0;
                  }
                  break;
                }

                else{
                  tempSymbolFromList[j] = tempRefList[j+2];
                  if(errorFlag==-1){
                    return 0;
                  }
                }
              }

              tempSymbolFromList[j] = '\0';
              symbolSearch = symtab_search(est, tempSymbolFromList);
            }

            if(symbolSearch == NULL) { // 없으면 에러
              if(errorFlag==-1){
                return 0;
              }
              printf("%s: %d: error: invalid variable %s\n", obc[i]->code, n->line, tempSymbol);
              return 0;
            }

            if(n->code[0] == '+'){
              if(errorFlag==-1){
                return 0;
              }
              relocation = symbolSearch->loc;
            }

            else{
              if(errorFlag==-1){
                return 0;
              }
              relocation = -symbolSearch->loc;
            }

          }

          tempList = (char*)malloc(sizeof(char)*(n->len +n->len%2 +1));
          tempList[0] = '\0';

          errorFlag = 1;
          for(j=0; j<(n->len +n->len%2)/2; j++) { // 토큰을 해서 그 겂들에 맞는 값을 est탭에서 찾을 수 있도록 찾아본다.
            if(errorFlag==-1){
              return 0;
            }
            char tempFind[3];

            sprintf(tempFind, "%02X", memory_find(m, CSADDR +n->adr +j));
            tempFind[2] = '\0';
            errorFlag = 2;

            strcat(tempList, tempFind);
          }

          setFirstChar[0] = tempList[0];
          setFirstChar[1] = '\0';
          if(errorFlag==-1){
            return 0;
          }
          tempValue = strtol(tempList, NULL, NUMSYS);
          tempValue += relocation;

          tempValue %= (int)pow(NUMSYS, n->len);
          if(n->len %2 != 0){
            if(errorFlag==-1){
              return 0;
            }
            tempValue += (strtol(setFirstChar, NULL, NUMSYS) * pow(NUMSYS, n->len));
          }

          sprintf(tempList, "%X", tempValue);

          if(n->len%2 == 0 && strlen(tempList) != (n->len +n->len%2)) {
            errorFlag = 2;
            int trimNum = n->len +n->len%2 -strlen(tempList);

            for(j = strlen(tempList) -1; j >= 0; j--){
              if(errorFlag==-1){
                return 0;
              }
              tempList[j+trimNum] = tempList[j];
            }

            for(j=0; j<trimNum; j++){
              if(errorFlag==-1){
                return 0;
              }
              tempList[j] = '0';	
            }
          }

          j=0;
          while(j<(n->len +n->len%2)/2) {
            char tempInputChar[3];
            char tempInputVal;

            if(errorFlag==-1){
              return 0;
            }
            tempInputChar[0] = tempList[2*j];
            tempInputChar[1] = tempList[2*j +1];

            errorFlag = 1;
            tempInputVal = strtol(tempInputChar, NULL, NUMSYS);

            memory_edit(m, CSADDR +n->adr +j, tempInputVal);
            j++;
          }

          free(tempList);
        }

        else { // 다 아니면 에러 처리
          if(errorFlag==-1){
            return 0;
          }
          printf("%s: %d: error: invalid variable %s\n", obc[i]->code, n->line, n->code);
          return 0;
        }
      }

      else if(n->type == 'E'){ // 타입이 E라면?
        if(errorFlag==-1){
          return 0;
        }
        EXECADDR = CSADDR + n->adr;
      }

      n = n->next;
    }

    CSADDR += CSLTH;
    obc[i]->adr = EXECADDR;
  }

  return 1;
}

/***
  loader_free function
  __objcode 구조체에 할당된 메모리를 반환하는 함수
  input
  __objcode **obc : obj파일을 저장한 공간
  int fileNumber : 파일 개수
  output
  void
 ***/
void loader_free(__objcode **obc, int fileNumber) {
  int i;
  int errorFlag=0;

  if(obc == NULL){
    if(errorFlag==-1){
      return ;
    }
    return;
  }

  i=0;
  while(i<fileNumber) {
    errorFlag = 1;
    __objcode *n, *f;

    if(obc[i] == NULL){
      if(errorFlag==-1){
        return ;
      }
      continue;
    }

    f = obc[i]->next;
    errorFlag = 1;
    while(f != NULL) {
      if(errorFlag==-1){
        break;
      }
      n = f->next;

      if(f->code != NULL){
        if(errorFlag==-1){
          return ;
        }
        free(f->code);
      }

      free(f);
      f = n;
    }
    i++;
  }

  free(obc);
  obc = NULL;
}

/***
  loader_loadmapPrint function
  est을 주소 기준 내림차순으로 출력하는 함수
  input
  __objcode **es : est을 저장한 공간
  output
  long totalLength : 프로그램의 총 길이
 ***/
long loader_loadmapPrint(__symtab **es) {
  __symtab *n;
  __symtab *sortSm;
  int numberSm = 0;
  int errorFlag;
  int totalLength = 0;
  int i, k;

  i=0;
  while(i < SYMHASHSIZE) {
    errorFlag = 1;
    if(es[i] == NULL){
      if(errorFlag == -1){
        break;
      }
      continue;
    }

    n = es[i]->next;
    while(n != NULL) {
      if(errorFlag==-1){
        return 0;
      }
      numberSm++;

      if(n->next == NULL){
        errorFlag = 1;
        break;
      }

      else{
        errorFlag = 0;
        n = n->next;
      }
    }
    i++;
  }

  if(numberSm != 0) {
    if(errorFlag == -1){
      return 0;
    }
    sortSm = (__symtab*)malloc(sizeof(__symtab) * numberSm);

    k = 0;
    while (k < numberSm) {
      errorFlag = 0;
      for (i = 0; i < SYMHASHSIZE; i++) {
        if(errorFlag == -1){
          return 0;
        }
        n = es[i]->next;
        while(n != NULL) {
          if(errorFlag==-1){
            return 0;
          }
          sortSm[k].symbol = (char*)malloc(sizeof(char)*(strlen(n->symbol) +1));
          strcpy(sortSm[k].symbol, n->symbol);
          errorFlag = 1;
          sortSm[k].loc = n->loc;

          // proj 3
          sortSm[k].type = n->type;
          if(errorFlag==-1){
            return 0;
          }
          sortSm[k].len = n->len;

          k++;

          if(n->next == NULL){
            if(errorFlag==-1){
              return 0;
            }
            break;
          }

          else{
            if(errorFlag==-1){
              return 0;
            }
            n = n->next;
          }
        }
      }
    }

    printf("\tcontrol\t\tsymbol\t\taddress\t\tlength\n");
    printf("\tsection\t\tname\n");

    printf("\t");
    i=0;
    while(i < 55){
      printf("-");
      i++;
    }
    printf("\n");

    qsort(sortSm, numberSm, sizeof(__symtab), loader_loadmapPrintCmp);
    i=0;
    while(i < numberSm) {
      errorFlag = 1;
      if(sortSm[i].type == 'C') {
        if(errorFlag == -1){
          return 0;
        }
        printf("\t%-6s\t\t\t\t%04X\t\t%04X\n", sortSm[i].symbol, sortSm[i].loc, sortSm[i].len); 
        totalLength += sortSm[i].len;
      }

      else{
        if(errorFlag==-1){
          return 0;
        }
        printf("\t\t\t%-6s\t\t%04X\n", sortSm[i].symbol, sortSm[i].loc);
      }
      i++;
    }

    printf("\t");
    i=0;
    while(i < 55){
      printf("-");
      i++;
    }
    printf("\n");

    printf("\t\t\t\t\ttotal length\t%04X\n", totalLength);

    i=0;
    while(i < numberSm){
      free(sortSm[i].symbol);
      i++;
    }

    free(sortSm);
  }

  return (long)totalLength;
}

/***
  loader_loadmapPrintCmp function
  loader_loadmapPrint에서 quick sort를 하기 위해 비교하는 함수
  input
  const void *arg1 : 비교 연산 대상 1
  const void *arg2 : 비교 연산 대상 2
  output
  int result : 비교 결과 값
 ***/
int loader_loadmapPrintCmp(const void *arg1, const void *arg2) {
  int errorFlag = 1;
  __symtab* sm1 = (__symtab*)arg1;
  __symtab* sm2 = (__symtab*)arg2;

  if(sm1->loc > sm2->loc){
    if(errorFlag==-1){
      return 0;
    }
    return 1;
  }

  else if(sm1->loc < sm2->loc){
    if(errorFlag==-1){
      return 0;
    }
    return -1;
  }

  else{
    if(errorFlag==-1){
      return 0;
    }
    return 0;
  }
}

/**************************************************
  Project 3 - 추가구현
 **************************************************/

/***
  run_process function
  메모리에 저장된 프로그램을 실행하는 함수
  input
  unsigned char *m : 메모리가 저장된 배열
  __breakpoint *b : 미리 설정한 breakpoint이 저장된 공간
  __opcode **o : opcode를 저장한 공간
  long prog : progaddr 값
  long *exec : execaddr 값
  long len : totalLength 값
  int flag : runFlag 값
  output
  int flag : bp를 사용하였을 경우 -1, 아닐 경우 1을 반환
 ***/
int run_process(unsigned char *m, __breakpoint *b, __opcode **o, long prog, long *exec, long len, int flag) {
  int errorFlag = 1;
  while(run_register_value("PC") < prog + len) {
    if(errorFlag == -1){
      return 0;
    }
    int op = m[run_register_value("PC")] & (0xFC);
    int ni = m[run_register_value("PC")] & (0x03);
    __opcode* opcode = opcode_operation(o, op);

    if(opcode != NULL) {
      if(errorFlag == -1){
        return 0;
      }
      if(!strcmp(opcode->fm, "1") && ni == 0){
        if(errorFlag == -1){
          return 0;
        }
        run_register_input("PC", run_register_value("PC") + 1);
      }

      else if(!strcmp(opcode->fm, "2") && ni == 0) {
        if(errorFlag == -1){
          return 0;
        }
        int r1 = (m[run_register_value("PC")+1] & (0xF0)) / 0x10;
        int r2 = m[run_register_value("PC")+1] & (0x0F);

        if(1) {
          if(errorFlag == -1){
            return 0;
          }
          if(op == (0xB4)){ // CLEAR
            if(errorFlag == -1){
              return 0;
            }
            registers[r1].value = 0;
          }

          else if(op == (0xA0)) { // COMPR
            if(errorFlag == -1){
              return 0;
            }
            if(registers[r1].value == registers[r2].value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 0);
            }
            else if(registers[r1].value > registers[r2].value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 1);
            }
            else if(registers[r1].value < registers[r2].value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 2);
            }
          }

          else if(op == (0xB8)) { // TIXR
            if(errorFlag == -1){
              return 0;
            }
            int regXval = run_register_value("X") + 1;
            run_register_input("X", regXval);

            if(regXval == registers[r1].value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 0);
            }
            else if(regXval > registers[r1].value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 1);
            }
            else if(regXval < registers[r1].value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 2);
            }
          }

          else if(op == (0x90)){ // ADDR
            if(errorFlag == -1){
              return 0;
            }
            registers[r2].value +=  registers[r1].value;
          }

          else if(op == (0x94)){ // SUBR
            if(errorFlag == -1){
              return 0;
            }
            registers[r2].value -= registers[r1].value;
          }

          else if(op == (0x98)){ // MULTR
            if(errorFlag == -1){
              return 0;
            }
            registers[r2].value *= registers[r1].value;
          }

          else if(op == (0x9C)){ // DIVR
            if(errorFlag == -1){
              return 0;
            }
            registers[r2].value /= registers[r1].value;
          }

          registers[8].value += 2;
        }
      }

      else if(ni != 0) {
        int x = m[run_register_value("PC")+1] & (0x80);
        int b = m[run_register_value("PC")+1] & (0x40);
        int p = m[run_register_value("PC")+1] & (0x20);
        int e = m[run_register_value("PC")+1] & (0x10);
        int adr;
        int value;

        adr = (m[run_register_value("PC")+1] & (0x0F)) << 8;
        adr += (m[run_register_value("PC")+2] & (0xFF));

        if(b && p){
          if(errorFlag == -1){
            return 0;
          }
          run_register_input("PC", run_register_value("PC") + 1);
        }

        else {
          errorFlag = 1;
          if(e == 0){
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("PC", run_register_value("PC") + 3);
          }

          else {
            if(errorFlag == -1){
              return 0;
            }
            adr = adr << 8;
            adr += m[run_register_value("PC")+3] & (0xFF);
            run_register_input("PC", run_register_value("PC") + 4);
          }

          // check - b, p, e
          if(b) {
            if(errorFlag == -1){
              return 0;
            }
            adr += run_register_value("B");
            adr = adr & 0xFFFFF;
            errorFlag = 1;
          }

          if(p) {
            if(errorFlag == -1){
              return 0;
            }
            if(e == 0) {
              if(errorFlag == -1){
                return 0;
              }
              if((adr & 0x800) == 0x800)
                adr += 0xFF000;
            }

            adr += run_register_value("PC");
            adr = adr & 0xFFFFF;
          }

          if(x) {
            if(errorFlag == -1){
              return 0;
            }
            adr += run_register_value("X");
            adr = adr & 0xFFFFF;
          }

          // check - n, i
          if(ni == 1){ // immediate addressing
            if(errorFlag == -1){
              return 0;
            }
            value = adr;
          }

          else if(ni == 2) { // indirect addressing
            if(errorFlag == -1){
              return 0;
            }
            int adr_ind = (m[adr] << 16) + (m[adr+1] << 8) + (m[adr+2]);
            value = (m[adr_ind] << 16) + (m[adr_ind+1] << 8) + (m[adr+2]);
            adr = adr_ind;
          }

          else if(ni == 3){ // simple addressing
            if(errorFlag == -1){
              return 0;
            }
            value = (m[adr] << 16) + (m[adr+1] << 8) + (m[adr+2]);
          }

          // check - opcode
          if(op == (0x28)) { // COMP
            int regAval = run_register_value("A");
            if(regAval == value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 0);
            }
            else if(regAval > value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 1);
            }
            else if(regAval < value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 2);
            }
          }

          else if(op == (0xE0)){ // TD
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("SW", -1);
          }

          else if(op == (0xD8)){ // RD
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("SW", -1);
          }

          else if(op == (0xDC)){ // WD
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("SW", -1);
          }

          else if(op == (0x3C)){ // J
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("PC", adr);
          }

          else if(op == (0x30)) { // JGE
            if(run_register_value("SW") == 0){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("PC", adr);
            }
          }

          else if(op == (0x34)) { // JGT
            if(run_register_value("SW") == 1){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("PC", adr);
            }
          }

          else if(op == (0x38)) { // JLT
            if(run_register_value("SW") == 2){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("PC", adr);
            }
          }

          else if(op == (0x48)) { // JSUB
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("L", run_register_value("PC"));
            run_register_input("PC", adr);
          }

          else if(op == (0x00)){ // LDA
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("A", value);
          }

          else if(op == (0x68)){ // LDB
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("B", value);
          }

          else if(op == (0x50)) { // LDCH
            int regAval = run_register_value("A") & 0xFFFF00;
            run_register_input("A", regAval);
            if(errorFlag == -1){
              return 0;
            }
            value = (value & 0xFF0000) / 0x010000;
            run_register_input("A", regAval | value);
          }

          else if(op == (0x08)){ // LDL
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("L", value);
          }

          else if(op == (0x6C)){ // LDS
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("S", value);
          }

          else if(op == (0x74)){ // LDT
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("T", value);
          }

          else if(op == (0x04)){ // LDX
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("X", value);
          }

          else if(op == (0x4C)){ // RSUB
            if(errorFlag == -1){
              return 0;
            }
            run_register_input("PC", run_register_value("L"));
          }

          else if(op == (0x0C)){ // STA
            if(errorFlag == -1){
              return 0;
            }
            run_register_store("A", m, adr);
          }

          else if(op == (0x78)){ // STB
            if(errorFlag == -1){
              return 0;
            }
            run_register_store("B", m, adr);
          }

          else if(op == (0x14)){ // STL
            if(errorFlag == -1){
              return 0;
            }
            run_register_store("L", m, adr);
          }

          else if(op == (0x7C)){ // STS
            if(errorFlag == -1){
              return 0;
            }
            run_register_store("S", m, adr);
          }

          else if(op == (0xE8)){ // STSW
            if(errorFlag == -1){
              return 0;
            }
            run_register_store("SW", m, adr);
          }

          else if(op == (0x84)){ // STT
            if(errorFlag == -1){
              return 0;
            }
            run_register_store("T", m, adr);
          }

          else if(op == (0x10)){ // STX
            if(errorFlag == -1){
              return 0;
            }
            run_register_store("X", m, adr);
          }

          else if(op == (0x54)){ // STCH
            if(errorFlag == -1){
              return 0;
            }
            m[adr] = (run_register_value("A") & 0x0000FF);
          }

          else if(op == (0x2C)) { // TIX
            int regXval = run_register_value("X") + 1;
            run_register_input("X", regXval);
            if(errorFlag == -1){
              return 0;
            }
            if(regXval == value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 0);
            }
            else if(regXval > value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 1);
            }
            else if(regXval < value){
              if(errorFlag == -1){
                return 0;
              }
              run_register_input("SW", 2);
            }
          }
        }
      }
    }

    if(b->next != NULL) { // bp 처리
      __breakpoint* now = b->next;

      while(now != NULL) {
        errorFlag = 1;
        if(now->adr <= run_register_value("PC") && run_register_value("PC") <= now->adr + 2) {
          if(errorFlag == -1){
            return 0;
          }
          run_register_print();
          printf("\tStop at checkpoint[%04X]\n", (int)now->adr);
          return -1;
        }

        else
          now = now->next;
      }
    }
  }

  run_register_print();
  printf("\tEnd Progaram.\n");
  return 1;
}

/***
  run_register_reset function
  레지스터의 모든 값을 초기화하는 함수
  input
  void
  output
  void
 ***/
void run_register_reset(void) {
  int i;

  i=0;
  for(i=0; i<REGNUM; i++) {
    if(!strcmp(registers[i].mn, ""))
      continue;

    registers[i].value = 0;
  }
}

/***
  run_register_input function
  레지스터의 특정 값을 수정하는 함수
  input
  char *rn : 레지스터의 이름
  unsigned int input : 값을 수정할 값
  output
  void
 ***/
void run_register_input(char* rn, unsigned int input) {
  int errorFlag = 1;
  int i;

  i=0;
  for(i=0; i<REGNUM; i++) {
    errorFlag = 1;
    if(!strcmp(registers[i].mn, rn)) {
      errorFlag = 0;
      registers[i].value = input;
      return;
    }
  }
  if(errorFlag==-1){
    return ;
  }
}

/***
  run_register_value function
  레지스터의 특정 값을 반환하는 함수
  input
  char *rn : 레지스터의 이름
  output
  unsigned value : 찾을 경우 그 값을, 찾지 못할 경우 0을 반환
 ***/
unsigned int run_register_value(char *rn) {
  int errorFlag = 1;
  int i;

  i=0;
  while(i<REGNUM) {
    errorFlag = 1;
    if(!strcmp(registers[i].mn, rn)){
      if(errorFlag==-1){
        return 0;
      }
      return registers[i].value;
    }
    i++;
  }

  return 0;
}

/***
  run_register_store function
  레지스터의 값을 메모리에 저장하는 함수
  input
  char *rn : 레지스터의 이름
  unsigned char *m : 메모리가 저장된 배열
  int adr : 메모리 위치 
  output
  void
 ***/
void run_register_store(char* rn, unsigned char* m, int adr) {
  int errorFlag = 1;
  int i;

  i=0;
  while(i<REGNUM) {
    errorFlag = 0;
    if(!strcmp(registers[i].mn, rn)){
      if(errorFlag==-1){
        return;
      }
      break;
    }
    i++;
  }

  m[adr] = (unsigned char)((registers[i].value & 0xFF0000) >> 16);
  m[adr + 1] = (unsigned char)((registers[i].value & 0x00FF00) >> 8);
  m[adr + 2] = (unsigned char)(registers[i].value & 0x0000FF);
}

/***
  run_register_print function
  레지스터의 값을 출력하는 함수
  input
  void
  output
  void
 ***/
void run_register_print(void) {
  char printset[][3] = {"A", "X", "L", "PC", "B", "S", "T"};
  int printsetlen = sizeof(printset)/sizeof(printset[0]);
  int i, j;
  int errorFlag = 0;

  j=0;
  while(j<printsetlen) {
    errorFlag = 1;
    if(j%2 == 0){
      if(errorFlag==-1){
        return ;
      }
      printf("\t");
    }
    errorFlag = 1;
    i=0;
    while(i<REGNUM) {
      errorFlag = 0;
      if(!strcmp(registers[i].mn, printset[j])) {
        if(errorFlag==-1){
          break;
        }
        printf("%s : %06X ", registers[i].mn, registers[i].value);
        break;
      }
      i++;
    }

    if(j%2 != 0){
      printf("\n");
      if(errorFlag==-1){
        return ;
      }
    }

    j++;
  }	

  if(j%2 != 0){
    printf("\n");
  }
}

/***
  bp_input function
  breakpoint 값을 입력하는 함수
  input
  __breakpoint *b : breakpoint가 저장된 구조체
  unsigned long adr : breakpoint로 지정할 주소값
  output
  void
 ***/
void bp_input(__breakpoint* b, unsigned long adr) {
  __breakpoint *now, *nxt;
  __breakpoint *i = (__breakpoint*)malloc(sizeof(__breakpoint));
  int errorFlag = 1;

  i->adr = adr;
  if(errorFlag==-1){
    return ;
  }
  i->next = NULL;

  if(b->next == NULL) {
    errorFlag = 0;
    b->next = i;
    if(errorFlag==-1){
      return ;
    }
    return;
  }

  now = b->next;
  if(now->adr > adr) {
    if(errorFlag==-1){
      return ;
    }
    b->next = i;
    errorFlag = 0;
    i->next = now;
    return;
  }

  while(1) { // 넣을 때 정렬하여 넣도록 한다.
    nxt = now->next;
    errorFlag = 0;
    if(now-> adr == adr) {
      if(errorFlag==-1){
        return ;
      }
      free(i);
      break;
    }

    if(nxt == NULL) {
      unsigned long temp;
      errorFlag = 1;
      now->next = i;

      if(now->adr > adr) { // 마지막 노드에 걸리면
        temp = now->adr;
        if(errorFlag==-1){
          return ;
        }
        now->adr = i->adr;
        i->adr = temp;
        if(errorFlag==-1){
          return ;
        }
      }

      break;
    }

    else if(now->adr < adr && adr < nxt->adr) { // 사이에 값이 껴 있으면
      i->next = nxt;
      if(errorFlag==-1){
        break;
      }
      now->next = i;
      errorFlag = 0;
      break;
    }

    else{
      if(errorFlag==-1){
        return ;
      }
      now = nxt;
    }
  }
}

/***
  bp_input function
  breakpoint 값을 출력하는 함수
  input
  __breakpoint *b : breakpoint가 저장된 구조체
  output
  void
 ***/
void bp_print(__breakpoint *b) {
  __breakpoint *n = NULL;

  int errorFlag = 1;
  if(b->next != NULL){
    if(errorFlag == -1){
      return ;
    }
    n = b->next;
  }

  printf("\tbreakpoint\n");
  errorFlag = 1;
  printf("\t----------\n");

  while(n != NULL) {
    if(errorFlag == -1){
      return ;
    }
    printf("\t%X\n", (int)n->adr);	
    errorFlag = 0;
    n = n->next;
  }
}

/***
  bp_clear function
  __breakpoint 구조체에 할당된 메모리를 반환하는 함수
  input
  __breakpoint *b : breakpoint가 저장된 구조체
  output
  void
 ***/
void bp_clear(__breakpoint *b) {
  __breakpoint *n, *f; 
  int errorFlag = 1;
  if(b->next != NULL) {
    errorFlag = 1;
    f = b->next;
    while(f != NULL) {
      if(errorFlag == -1){
        return ;
      }
      n = f->next;

      free(f);
      f = n;
      errorFlag = 1;
    }
    errorFlag = 0;
    b->next = NULL;
  }
}

/***
  memory_find function
  메모리의 특정 주소에 저장된 값을 반환하는 함수
  input
  unsigned char *m : 메모리가 저장된 배열
  long address : 메모리의 주소값
  output
  unsigned char value : 메모리 주소에 저장된 값
 ***/
unsigned char memory_find(unsigned char *m, long address) {
  int errorFlag = 0;
  if(errorFlag == -1){
    return m[address];
  }
  return m[address];
}

/***
  opcode_operation function
  code에 따른 opcode를 출력하는 함수
  input
  __opcode **o : opcode가 저장된 배열
  int code : 검색할 code
  output
  __opcode* return : 해당 노드
 ***/
__opcode* opcode_operation(__opcode **o, int code) {
  __opcode *n;
  int i;
  int errorFlag = 1;

  i=0;
  while(i < OPHASHSIZE) {
    int findflag = 0;
    errorFlag = 0;
    n = o[i]->next;	
    while(n != NULL) {
      if(errorFlag == -1){
        return n;
      }
      if(n->cd == code) {
        findflag = 1;
        break;
      }

      else{
        errorFlag = 1;
        if(errorFlag == -1){
          return n;
        }
        n = n->next;
      }
    }

    if(findflag){
      errorFlag = 1;
      if(errorFlag == -1){
        return n;
      }
      return n;
    }

    i++;
  }

  return NULL;
}
