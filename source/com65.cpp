
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <random>
#include "../include/tokenize.hpp"
#include "../include/parse.hpp"
#include "../include/file_controller.hpp"

Node *expr();
Node *stmt();
Node *add();

// ローカル関数リスト
std::vector<LVar*> locals;
// 関数リスト
std::vector<LFunc*> funcs;
// tokenizeの結果がここに入る
std::vector<Token> tokens;
int pos = 0;
int random_label = 0;
bool array_exist = false;




LVar *find_lvar(char *str,int len){
    for(int i=0;i<locals.size();i++){
        LVar *local = locals[i];
        if(local->len==len&&!memcmp(str,local->name,local->len)){
            return local;
        }
    }
    return NULL;
}


LFunc *find_func(Token *tok){
    for(int i=0;i<funcs.size();i++){
        LFunc *func = funcs[i];
        if(func->len==tok->len&&!memcmp(tok->str,func->name,func->len)){
            return func;
        }
    }
    return NULL;
}


Node *new_node(int ty,Node *lhs, Node *rhs){
    Node *node = (Node *)malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}
 
Node *new_node_num(int val){
    Node *node = (Node *)malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    node->lhs = NULL;
    node->rhs = NULL;
    return node;
}


bool consume(char *op){
    if(memcmp(tokens[pos].str,op,tokens[pos].len)){
        //printf("false %s\n",tokens[pos].str);
        return false;
    }
    pos++;
    return true;
}

bool at_eof(){
    return tokens[pos].ty == TK_EOF;
}

int check_func_type(int type){
    switch(type){
        case TK_CHAR:
            return ND_CHAR;
        case TK_SHORT:
            return ND_SHORT;
    }
    return 0;
}

void expect(char *p){
    if(!memcmp(tokens[pos].str,p,tokens[pos].len)){
        return;
    }
    printf("error expect %s is not %s\n",p,tokens[pos].str);
    exit(1);
}

std::vector<Node*> call_arrgument(){
    std::vector<Node*> stmts;
    char cr[] = ")",com[] = ",";
    while(memcmp(tokens[pos].str,cr,1)){
        LVar *arrgu = find_lvar(tokens[pos].str,tokens[pos].len);
        if(arrgu){
            // 変数だった時の処理
            Node *buf;
            buf = (Node*)malloc(sizeof(Node));
            buf->ty = ND_LVAR;
            buf->str = arrgu->name;
            buf->len = arrgu->len;
            buf->offset = arrgu->offset;
            stmts.push_back(buf);
        }else{
            // 即値だった時の処理
            Node *buf;
            buf = (Node*)malloc(sizeof(Node));
            buf->ty = ND_NUM;
            buf->val = tokens[pos].val;
            stmts.push_back(buf);
        }
        pos++;
        if(!memcmp(tokens[pos].str,cr,1)){
            break;
        }else if(!memcmp(tokens[pos].str,com,1)){
            pos++;
            continue;
        }else{
            printf("err call func arrgument\n");
            exit(1);
        }
    }

    return stmts;
}

Node *primary(){

    if(consume((char*)"(")){
        Node *node = expr();
        if(consume((char*)")")){
            return node;
        }
    }

    if(consume((char*)"{")){
        Node *node = (Node*)malloc(sizeof(Node));
        Node *buf = (Node*)malloc(sizeof(Node));
        node->ty = ND_IMM_ARRAY;
        node->lhs = buf;
        int len = 0;
        while(1){
            buf->rhs = (Node*)malloc(sizeof(Node));
            buf->rhs = add();
            len++;
            buf->lhs = (Node*)malloc(sizeof(Node));
            buf = buf->lhs;
            if(consume((char*)"}")){break;}
            pos++;
        }
        node->val = len;
        return node;
    }

    if(tokens[pos].ty == TK_NUM){
        return new_node_num(tokens[pos++].val);
    }
    
    if(tokens[pos].ty==TK_IDENT){
        Node *node = (Node*)malloc(sizeof(Node));

        // 関数呼び出し or 定義
        char cl[] = "(",cr[] = ")",bl[]="{",br[]="}";
        // ( の判定
        if(!memcmp(tokens[pos+1].str,cl,1)){
            // IDENT and cl plus
            node->ty = ND_FUNC;
            node->str = tokens[pos].str;
            node->len = tokens[pos].len;
            pos+=2;
            //引数処理
            node->stmts = call_arrgument();
            // ) の判定
            if(!memcmp(tokens[pos].str,cr,1)){
                pos++;
                return node;
            }
            printf("error function call\n");
            exit(1);
        }

        // 変数呼び出し
        node->ty = ND_LVAR;
        node->str = (char*)malloc(sizeof(char));
        node->str = tokens[pos].str;
        node->len = tokens[pos].len;
        LVar *lvar = find_lvar(tokens[pos].str,tokens[pos].len);
        pos++;
        // 配列か否かの判定
        if(consume((char*)"[")){
            node->ty = ND_ARRAY;
            node->lhs = add();
            if(!consume((char*)"]")){
                std::cout<<"error array miss ']' "<<std::endl;
                exit(1);
            }
            
        }

        // ノードにオフセットを入れる
        if(lvar){
            node->offset = lvar->offset;
        }else{
           printf("not found lvar %s\n",tokens[pos].str);
           exit(1);
        }
        return node;
    }

    printf("%s\n",tokens[pos].str);

    printf("error gen node\n");
    exit(1);
}

Node *unary(){

    if(consume((char*)"+")){
        return primary();
    }
    if(consume((char*)"-")){
        return new_node('-',new_node_num(0),primary());
    }
    if(consume((char*)"*")){
        Node *buf = primary();
        buf->ty = ND_POINTER;
        return buf;
    }
    if(consume((char*)"&")){
        return new_node(ND_ADDER,primary(),NULL);
    }
    Node *p = primary();
    return p;
}

Node *mul(){

    Node *node = unary();
    
    for(;;){
        if(consume((char*)"*")){
            node = new_node('*',node,unary());
        }else if(consume((char*)"/")){
            node = new_node('/',node,unary());
        }else{
            return node;
        }
    }
}

Node *add(){

    Node *node = mul();
    
    for(;;){
        if(consume((char*)"+")){
            node = new_node('+',node,mul());
        }else if(consume((char*)"-")){
            node = new_node('-',node,mul());
        }else{
            return node;
        }
    }

}

Node *relational(){

    Node *node = add();
    
    char setle[] = "<=",setre[] = ">=";
    for(;;){
        if(consume((char*)"<")){
            node = new_node(ND_SETL,node,add());
        }else if(consume((char*)">")){
            node = new_node(ND_SETL,add(),node);
        }else if(consume(setle)){  // <=
            node = new_node(ND_SETLE,node,add());
        }else if(consume(setre)){  // >=
            node = new_node(ND_SETLE,add(),node);
        }else{
            return node;
        }
    }

}

Node *equality(){

    Node *node = relational();

    for(;;){
        if(tokens[pos].ty==TK_SETE){  // == 
            pos++;
            node = new_node(ND_SETE,node,relational());
        }else if(tokens[pos].ty==TK_SETNE){ // !=
            pos++;
            node = new_node(ND_SETNE,node,relational());
        }else{
            return node;
        }
    }
}


Node *assign(){
    // ポインタへの代入。ただし、cpu設計的に1byteの値しか扱えないため、変数の値が0xff以下でないとダメ。
    if(consume((char*)"*")){
        Node *node = equality();
        if(consume((char*)"=")){
            node = new_node(ND_POINTER_ASSIGN,node,equality());
        }
        return node;
    }
    // 0xffより上の値のメモリに代入したい時にはこちらが使われる。
    if(consume((char*)"[")){
        Node *node = (Node*)malloc(sizeof(Node));
        // とりあえず入れてる。別になくてもいい。
        node->ty = ND_NUM;
        node->val = tokens[pos].val;
        pos++;
        if(consume((char*)"]")){
            if(consume((char*)"=")){
                node = new_node(ND_POINTER_ASSIGN_ABS,node,equality());
                return node;
            }
            printf("error pointer assign abs\n");
            exit(1);
        }
    }

    Node *node = equality();
    if(consume((char*)"=")){
        node = new_node(ND_ASSIGN,node,equality());
    }

    return node;
}

Node *expr(){

    char array_l[] = "[";

    // 変数宣言
    if(check_func_type(tokens[pos].ty)){
        // 型種類処理
        LVar *buf_lvar = (LVar*)malloc(sizeof(LVar));
        buf_lvar->lvar_type = (Type*)malloc(sizeof(Type));
        buf_lvar->lvar_type->type = check_func_type(tokens[pos].ty);
        // 変数登録
        pos++;
        if(tokens[pos].ty!=TK_IDENT){
            printf("error expr. ident tokens not found.");
        }
        // 変数の名前
        buf_lvar->name = (char*)malloc(sizeof(char)*tokens[pos].len);
        strncpy(buf_lvar->name,tokens[pos].str,tokens[pos].len);
        buf_lvar->len = tokens[pos].len;
        if(!locals.size()){
            buf_lvar->offset = 0x206;
        }else{
            if(!memcmp(array_l,tokens[pos+1].str,1)){
                if(!array_exist){
                    array_exist = true;
                    buf_lvar->offset = 0x0700;
                }else{
                    std::cout << "two array can not exist" << std::endl;
                    exit(1);  
                }
            }else{
                if(locals[(locals.size()-1)]->offset>=0x0700){
                    buf_lvar->offset = locals[(locals.size()-2)]->offset + 1;
                }else{
                    buf_lvar->offset = locals[(locals.size()-1)]->offset + 1;
                }
            }
        }
        locals.push_back(buf_lvar);
        return assign();
    }else{
        // 変数宣言でない処理。
        Node *p = (Node*)malloc(sizeof(Node));
        p = assign();
        return p;
    }
}


Node *stmt(){
    Node *node;

    if(tokens[pos].ty==TK_LOOP){
        node = (Node*)malloc(sizeof(Node));
        node->ty = ND_LOOP;
        pos++;
        expect((char*)";");
        pos++;
        return node;
    }

    if(tokens[pos].ty==TK_RETURN){
        node = (Node*)malloc(sizeof(Node));
        node->ty = ND_RETURN;
        pos++;
        node->lhs = expr();
        expect((char*)";");
        pos++;
        return node;
    }
    
    if(tokens[pos].ty==TK_IF){
        pos++;
        if(consume((char*)"(")){
            node = (Node*)malloc(sizeof(Node));
            node->ty = ND_IF;
            node->lhs = expr();
            if(consume((char*)")")){
                Node *else_node = (Node*)malloc(sizeof(Node));
                else_node = stmt();
                if(tokens[pos].ty==TK_ELSE){
                    pos++;
                    node->rhs = new_node(ND_ELSE,else_node,stmt());
                    return node;
                }
                
                node->rhs = else_node;
                return node; 
            }else{
                std::cout << "error if struct" << std::endl;
            }
        }
    }

    if(tokens[pos].ty==TK_WHILE){
        pos++;
        if(consume((char*)"(")){
            node = (Node*)malloc(sizeof(Node));
            node->ty = ND_WHILE;
            node->lhs = (Node*)malloc(sizeof(Node));
            node->lhs = expr();
            if(consume((char*)")")){
                node->rhs = (Node*)malloc(sizeof(Node));
                node->rhs = stmt();
                return node; 
            }
        }
    }
    
    if(consume((char*)"{")){
        // なぜかこのcoutがないとバグる
        //std::cout << "" << std::endl;
        node = (Node*)malloc(sizeof(Node));
        node->ty = ND_BLOCK;
        Node *buf = (Node*)malloc(sizeof(Node));
        node->lhs = buf;
        int len = 0;
        std::vector<Node*> buf_vector;
        while(!consume((char*)"}")){
            buf->rhs = stmt();
            buf->lhs = (Node*)malloc(sizeof(Node));
            buf = buf->lhs;
            len++;
        }
        node->val = len;
        return node;
    }
    
    node = expr();
    expect((char*)";");
    pos++;
    return node;
}



void program(){
    char br[] = "}";
    while(!at_eof()){
        // 関数の型処理
        if(check_func_type(tokens[pos].ty)){
            LFunc *func = (LFunc*)malloc(sizeof(LFunc));
            func->lvar_type = (Type*)malloc(sizeof(Type));
            func->lvar_type->type = check_func_type(tokens[pos].ty);
            pos++;
        // 関数本体
            if(tokens[pos].ty==TK_IDENT){
                //関数名取得
                func->name = (char*)malloc(tokens[pos].len+1);
                strncpy(func->name,tokens[pos].str,tokens[pos].len+1);
                //func->name = tokens[pos].str;
                func->len = tokens[pos].len;
                pos++;
                if(consume((char*)"(")){
                    // 引数処理
                    if(consume((char*)")")){
                        if(consume((char*)"{")){
                            func->code = (Node*)malloc(sizeof(Node));
                            func->val = 0;
                            Node *buf;
                            buf = func->code;
                            std::vector<Node*> lcode;
                            while (memcmp(br,tokens[pos].str,1)){ 
                                buf->lhs = (Node*)malloc(sizeof(Node));
                                buf->rhs = stmt();
                                buf = buf->lhs;
                                func->val++;
                            }

                            //std::copy(lcode.begin(),lcode.end(),back_inserter(func->code));
                            // どちらもvectorのコピー　挙動が違うので今後のために残す
                            //func->lvar_locals = locals;
                            /*
                            for(int i=0;i<locals.size();i++){
                                func->lvar_locals.push_back(locals[i]);
                            }
                            */
                            //std::copy(locals.begin(),locals.end(),back_inserter(func->lvar_locals));
                            
                            //locals vector のreset
                            //std::vector<LVar*> buf;
                            //locals = buf;
                            funcs.push_back(func);
                            pos++;
                            continue;
                        }
                    }
                }
            }
        }else{
            printf("erro functional define\n");
            exit(1);
        }
        
    }

}

int back_random(){
    random_label += 87;
    return random_label;
}

void gen(Node *node){

    //std::cout << "gen start" << std::endl;

    if(node->ty==ND_POINTER){
        printf("    ldx $%x\n",node->offset);
        printf("    lda [#$00]\n");
        printf("    pha\n");
        return;
    }

    if(node->ty==ND_ADDER){
        printf("    lda #$%x\n",node->lhs->offset);
        printf("    pha\n");
        return;
    }
           
    if(node->ty==ND_FUNC){
        for(int i=node->stmts.size()-1;i>=0;i--){
            Node *p = node->stmts[i];
            if(p->ty==ND_NUM){
                printf("    lda #$%x\n",p->val);
                printf("    pha\n");
            }else if(p->ty==ND_LVAR){
                printf("    lda $%d\n",p->offset);
                printf("    pha\n");
            }else{
                printf("err gen code call func\n");
                exit(1);
            }
        }
        printf("    call _Z3%.*sv\n",node->len,node->str);
        return;
    }
    
    if(node->ty==ND_BLOCK){
        Node *buf = (Node*)malloc(sizeof(Node));
        buf = node->lhs;
        for(int i=0;i<node->val;i++){
            gen(buf->rhs);
            buf = buf->lhs;
        }
        return;
    }
    
    if(node->ty == ND_LOOP){
        int L_LOOP = back_random();
        printf(".%d_LOOP",L_LOOP);
        printf("    jmp %d_LOOP\n",L_LOOP);
        return;
    }

    if(node->ty == ND_RETURN){
        gen(node->lhs);
        // aレジスタを返り値として設定する
        printf("    pla\n");
        printf("    rti\n");
        return;
    }
    
    if(node->ty == ND_IF){
        
        int L_IF = back_random();
        gen(node->lhs);
        printf("    pla\n");
        // sta だとゼロフラッグがうごかないので一回メモリに動かしてからldaしてる。ldaはゼロフラグに依存関係がある。
        printf("    sta $0\n");
        printf("    lda $0\n");
        
        if(node->rhs->ty==ND_ELSE){
            printf("    beq  %d_IF_jmp_else\n",L_IF);
            printf("    jmp %d_if_execute\n",L_IF);
            printf(".%d_IF_jmp_else",L_IF);
            printf("    jmp .%delse\n",L_IF);
            printf(".%d_if_execute",L_IF);
            gen(node->rhs->lhs);
            printf("    jmp %dend\n",L_IF);
            printf(".%delse",L_IF);
            gen(node->rhs->rhs);
        }else{
            printf("    beq  %d_IF_jmp_end\n",L_IF);
            printf("    jmp %d_if_execute\n",L_IF);
            printf(".%d_IF_jmp_end",L_IF);
            printf("    jmp %dend\n",L_IF);
            printf(".%d_if_execute",L_IF);
            gen(node->rhs);
        }
        printf(".%dend",L_IF);
        //これがないとラベルが重複することがありバグる
        //処理自体に意味はない
        printf("    lda #$0\n");
        return;
    }
    
    if(node->ty == ND_WHILE){
        int L_WHILE = back_random();
        printf(".%d_WHILE_begin",L_WHILE);
        //条件式
        gen(node->lhs);
        printf("    pla\n");
        // sta だとゼロフラッグがうごかないので一回メモリに動かしてからldaしてる。ldaはゼロフラグに依存関係がある。
        printf("    sta $0\n");
        printf("    lda $0\n");
        printf("    beq  %d_WHILE_end\n",L_WHILE);
        //演算
        gen(node->rhs);
        printf("    jmp %d_WHILE_begin\n",L_WHILE);
        printf(".%d_WHILE_end",L_WHILE);
        return;
    }


    if(node->ty == ND_NUM){
        printf("    lda #$%x\n",node->val);
        printf("    pha\n");
        return;
    }else if(node->ty==ND_ARRAY){
        gen(node->lhs);
        printf("    pla\n");
        printf("    sta $0\n");
        printf("    ldx $0\n");
        printf("    lda [#$%x]\n",node->offset);
        printf("    pha\n");
        return;
    }else if(node->ty==ND_IMM_ARRAY){
        Node *buf = node->lhs;
        for(int i=0;i<node->val;i++){
            //std::cout << "ND_IMM_ARRAY start" << std::endl;
            gen(buf->rhs);
            //std::cout << "ND_IMM_ARRAY end" << std::endl;
            buf = buf->lhs;
        }
        return;
    }else if(node->ty==ND_LVAR){
        printf("    lda $%x\n",node->offset);
        printf("    pha\n");
        return;
    }else if(node->ty==ND_ASSIGN){ 

        if(node->lhs->ty==ND_LVAR){
            gen(node->rhs);
            printf("    pla\n");
            printf("    sta $%x\n",node->lhs->offset);
            return;
        }

        if(node->rhs->ty==ND_IMM_ARRAY){
            int L_ARRAY;
            L_ARRAY = back_random();
            gen(node->rhs);
            gen(node->lhs->lhs);
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    ldx $0\n");
            printf("    dex\n");
            printf(".%dassgin_array", L_ARRAY);
            printf("    pla\n");
            printf("    sta [#$%x]\n", node->lhs->offset);
            printf("    dex\n");
            printf("    bne %dassgin_array\n", L_ARRAY);
            // x == 0 になっても代入してほしいのでこれがいる
            printf("    pla\n");
            printf("    sta $%x\n", node->lhs->offset);
            return;
        }

        std::cout << "node->rhs is not LVAR or array" << std::endl;
        exit(1);
    }else if(node->ty==ND_POINTER_ASSIGN){ // この代入ポインタの値は0xff以下であるという前提である。
        gen(node->rhs);
        printf("    pla\n");
        printf("    ldx $%x\n",node->lhs->offset);
        printf("    sta [#$00]\n");
        return;
    }else if(node->ty==ND_POINTER_ASSIGN_ABS){ // この代入ポインタの値は0xff以下であるという前提である。
        gen(node->rhs);
        printf("    pla\n");
        printf("    sta $%x\n",node->lhs->val);
        return;
    }

    // pointer との違い
    /*
    普通の assign は普通に変数のoffsetの所に値を入れるが
    pointer_assignは変数の値をoffsetと考えその部分に値を入れる。
    */
    gen(node->lhs);
    gen(node->rhs);



    switch(node->ty){

        case '+':
            printf("    clc\n");
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    pla\n");
            printf("    adc $0\n");
            break;
        case '-':
            printf("    sec\n");
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    pla\n");
            printf("    sbc $0\n");
            break;
        case '*':
            int L_MUL;
            L_MUL = back_random();
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    ldx $0\n");
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    lda #$0\n");
            printf("    clc\n");
            printf(".%d_mul",L_MUL);
            printf("    adc $0\n");
            printf("    dex\n");
            printf("    bne %d_mul\n",L_MUL);
            break;
        // 割り算だけまだ実装してないよ♡
        case '/':
            printf("    mov rdx, 0\n");
            printf("    div rdi\n");
            break;

        case ND_SETE: // == 
            int L_SETE;
            L_SETE = back_random();
            printf("    sec\n");
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    pla\n");
            printf("    sbc $0\n");

            printf("    beq %d_set_1\n",L_SETE);

            printf(".%d_set_0",L_SETE);
            printf("    lda #$0\n");
            //printf("    pha\n");
            printf("    jmp %d_sete_end\n",L_SETE);
            printf(".%d_set_1",L_SETE);
            printf("    lda #$1\n");
            //printf("    pha\n");

            printf(".%d_sete_end",L_SETE);
            break;

        case ND_SETL:  // <
            int L_SETL;
            L_SETL = back_random();
            printf("    sec\n");
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    pla\n");
            printf("    sbc $0\n");

            printf("    jmp %d_set_1\n",L_SETL);

            printf(".%d_set_0",L_SETL);
            printf("    lda #$0\n");
            //printf("    pha\n");
            printf("    jmp %d_sete_end\n",L_SETL);
            printf(".%d_set_1",L_SETL);
            printf("    lda #$1\n");
            //printf("    pha\n");

            printf(".%d_sete_end",L_SETL);
            break;
        // とりあえず実装しない。
        case ND_SETLE: // <=
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_SETNE: // !=
            int L_SETNE;
            L_SETNE = back_random();
            printf("    sec\n");
            printf("    pla\n");
            printf("    sta $0\n");
            printf("    pla\n");
            printf("    sbc $0\n");

            printf("    bne %d_set_1\n",L_SETNE);

            printf(".%d_set_0",L_SETNE);
            printf("    lda #$0\n");
            //printf("    pha\n");
            printf("    jmp %d_sete_end\n",L_SETNE);
            printf(".%d_set_1",L_SETNE);
            printf("    lda #$1\n");
            //printf("    pha\n");

            printf(".%d_sete_end",L_SETNE);
            break;
    }

    printf("    pha\n");
}



int main(int argc,char **argv){
    srand((unsigned)time(NULL));
    random_label = rand();
    if(argc != 2){
        fprintf(stderr,"引数の個数が正しくありません\n");
        return 1;
    }
    
    char *code = read_file(argv[1]);

    //トークンナイズ
    tokenize(code);
            
    /*
    for(int i=0;i<tokens.size();i++){
        std::cout << "------------------------" << std::endl;
        printf("ty -> %d\n",tokens[i].ty);
        printf("val -> %d\n",tokens[i].val);
        printf("str -> %s\n",tokens[i].str);
        printf("len -> %d\n",tokens[i].len);
        std::cout << "------------------------" << std::endl;
    }
    */
    program();
    LFunc *main_func;
    char main_str[] = "main";

    // 抽象構文木を下りながらコード生成
    for(int i=0;i<funcs.size();i++){
        LFunc *func = funcs[i];
        if(!memcmp(func->name,main_str,func->len)){main_func = func;continue;}
        printf(".%.*s",func->len,func->name);
        Node *code = func->code;
        for(int k=0;k<func->val;k++){
            gen(code->rhs);
            code = code->lhs;
        }
        printf("\n");
    }

    printf(".%s",main_str);
    Node *maincode = main_func->code;
    for(int i=0;i<main_func->val;i++){
        gen(maincode->rhs);
        maincode = maincode->lhs;
    }
    
    return 0;
}