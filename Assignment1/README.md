### Compiler Design Assignment1 - Lexical Analysis

**Student ID : 2020028377**
**Student Name : Mun GyeongTae**

과제 1 번의 경우 2가지 방법을 사용하여 Lexical Analysis를 수행하는 것입니다.
* **Method 1 : 직접 c코드를 이용하여 FSM 구현.**
* **Method 2 : flex를 이용하여 Lexical Analysis 구현**

아래에서 각 방법 별로 어떻게 구현했는지 설명드리도록 하겠습니다. 

***

### Method 1. cminus - C implementation

첫 번째 방법의 경우 주어진 c 코드로 FSM을 구현하는 것입니다. 

구현 전, 과제의 명세에 따라 몇 가지 파일을 수정하도록 하겠습니다. 바꿀 파일들은 각각 **globals.h**, **main.c**, **scan.c**, **util.c** 네 개 입니다. 각각의 파일들은 명세에 따라 고치면 됩니다.

명세에 따라 파일을 고치고 난 후, scan.c를 FSM에 맞게 고쳐주시면 됩니다. 여기서 중요한 건, **STATE**와 **SYMBOL**을 잘 보고 이에 맞게 구현하는게 중요합니다. 대부분의 심볼과 state의 경우 만들어져 있는 스켈레톤 코드를 따라가면 됩니다.

* 먼저 single token으로 알 수 있는 SYMBOL들이 있습니다.
* 그 다음 ==, !=, >=, <=과 같이 두 개의 토큰이 있어야 결정할 수 있는 SYMBOL들이 있습니다.
* 두 개의 토큰으로 결정되는 SYMBOL의 경우엔 STATE를 활용하여 구현할 계획입니다.
* 과제에서 명시한 12개의 STATE와 더불어, comment를 위한 한 개의 state를 추가로 사용할 계획입니다. (이유는 후술합니다.)
* 먼저 scan.c 파일을 뜯어봅시다.
    ```c
    switch (state) { 
        case START:
          if (isdigit(c))
            state = INNUM;
          else if (isalpha(c))
            state = INID;
            ...
          else if (c == EOF) {
            save = FALSE;
            state = DONE;
            currentToken = ENDFILE;
          }
          else { 
            state = DONE;
            ...
    ``` 
* scan.c 파일을 뜯어보면 getToken이라는 함수 안에 다음과 같은 내용이 있습니다.
* 이때 state따라 save, currentToken, state를 바꾸는 것을 알 수 있습니다.
* 또, unGetNextchar()함수도 있는데, 이는 다음 charcter를 받지않고 다시 백업으로 돌아가는 역할을 해줍니다