//
// Created by lucas on 22/05/2026.
//
/*
 * DCPU-16 Assembler / Disassembler  –  C port
 *
 * Usage:
 *   dcpu16 assemble <input.asm> [output.bin]
 *   dcpu16 disasm   <input.bin>
 *   dcpu16 pretty   <input.asm>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

/* ─── limits ─────────────────────────────────────────────── */
#define SIZE       0x10000
#define MAX_TOKENS 131072
#define MAX_LABELS 4096
#define MAX_INSTRS 65536
#define MAX_WORDS  65536
#define MAX_STR    128
#define MAX_DATA   256   /* words per DAT */

/* ─── opcode tables ──────────────────────────────────────── */
typedef struct { const char *name; int code; } OE;
static const OE BOP[]={
  {"SET",0x01},{"ADD",0x02},{"SUB",0x03},{"MUL",0x04},{"MLI",0x05},
  {"DIV",0x06},{"DVI",0x07},{"MOD",0x08},{"MDI",0x09},{"AND",0x0a},
  {"BOR",0x0b},{"XOR",0x0c},{"SHR",0x0d},{"ASR",0x0e},{"SHL",0x0f},
  {"IFB",0x10},{"IFC",0x11},{"IFE",0x12},{"IFN",0x13},{"IFG",0x14},
  {"IFA",0x15},{"IFL",0x16},{"IFU",0x17},{"ADX",0x1a},{"SUX",0x1b},
  {"STI",0x1e},{"STD",0x1f},{NULL,0}};
static const OE SOP[]={
  {"JSR",0x01},{"INT",0x08},{"IAG",0x09},{"IAS",0x0a},{"RFI",0x0b},
  {"IAQ",0x0c},{"HWN",0x10},{"HWQ",0x11},{"HWI",0x12},{NULL,0}};
static const OE COP[]={{"NOP",0x0000},{"BRK",0x0040},{"RFI",0x0160},{NULL,0}};
static const OE REGS[]={{"A",0},{"B",1},{"C",2},{"X",3},{"Y",4},{"Z",5},{"I",6},{"J",7},{NULL,0}};
static const OE DSTC[]={{"PUSH",0x18},{"PEEK",0x19},{"SP",0x1b},{"PC",0x1c},{"EX",0x1d},{NULL,0}};
static const OE SRCC[]={{"POP",0x18},{"PEEK",0x19},{"SP",0x1b},{"PC",0x1c},{"EX",0x1d},{NULL,0}};

static int lu(const OE *t, const char *n){
    for(int i=0;t[i].name;i++) if(!strcmp(t[i].name,n)) return t[i].code;
    return -1;}
static const char *rlu(const OE *t, int c){
    for(int i=0;t[i].name;i++) if(t[i].code==c) return t[i].name;
    return NULL;}

/* ─── lexer ──────────────────────────────────────────────── */
typedef enum{TK_LB,TK_RB,TK_PLUS,TK_INC,TK_DEC,TK_AT,
             TK_LABEL,TK_ID,TK_NUM,TK_STR,TK_EOF} TT;

/* For TK_STR we store individual char values as a sequence of TK_NUM tokens */
typedef struct{ TT t; char s[MAX_STR]; int32_t v; } Tok;

typedef struct{ const char *src; int p,line; char pfx[MAX_STR]; } Lex;

static void strupr2(char *d,const char *s){
    int i;for(i=0;s[i]&&i<MAX_STR-1;i++) d[i]=(char)toupper((unsigned char)s[i]);
    d[i]=0;}

static void lerr(Lex *l,const char *m){
    fprintf(stderr,"Lex error line %d: %s\n",l->line,m);exit(1);}

/* Tokenise src into pre-allocated array; returns count */
static int tokenise(const char *src, Tok *out, int cap){
    Lex l; l.src=src; l.p=0; l.line=1; l.pfx[0]=0;
    int n=0;
    while(n<cap-1){
        const char *s=src; int p=l.p;
    skip:
        while(s[p]==' '||s[p]=='\t'||s[p]=='\r'||s[p]==',') p++;
        if(s[p]==';'){while(s[p]&&s[p]!='\n')p++;}
        if(s[p]=='\n'){l.line++;p++;goto skip;}
        l.p=p;
        Tok tk; memset(&tk,0,sizeof(tk));
        if(!s[p]){tk.t=TK_EOF;out[n++]=tk;break;}
        if(s[p]=='+'&&s[p+1]=='+'){tk.t=TK_INC;l.p=p+2;out[n++]=tk;continue;}
        if(s[p]=='-'&&s[p+1]=='-'){tk.t=TK_DEC;l.p=p+2;out[n++]=tk;continue;}
        if(s[p]=='['){tk.t=TK_LB;l.p=p+1;out[n++]=tk;continue;}
        if(s[p]==']'){tk.t=TK_RB;l.p=p+1;out[n++]=tk;continue;}
        if(s[p]=='+'){tk.t=TK_PLUS;l.p=p+1;out[n++]=tk;continue;}
        if(s[p]=='@'){tk.t=TK_AT;l.p=p+1;out[n++]=tk;continue;}
        /* char literal */
        if(s[p]=='\''){
            p++; tk.t=TK_NUM; tk.v=(unsigned char)s[p]; p++;
            if(s[p]=='\'')p++; l.p=p; out[n++]=tk; continue;}
        /* string literal → emit one TK_STR then individual TK_NUM per char */
        if(s[p]=='"'){
            p++; tk.t=TK_STR; tk.s[0]=0; out[n++]=tk; /* sentinel */
            while(s[p]&&s[p]!='"'){
                Tok ct; ct.t=TK_NUM; ct.v=(unsigned char)s[p++];
                ct.s[0]=0; out[n++]=ct;}
            if(s[p]=='"')p++; l.p=p; continue;}
        /* number */
        if(isdigit((unsigned char)s[p])||(s[p]=='-'&&isdigit((unsigned char)s[p+1]))){
            int neg=(s[p]=='-'); if(neg)p++;
            long long v=0;
            if(s[p]=='0'&&(s[p+1]=='x'||s[p+1]=='X')){
                p+=2;
                while(isxdigit((unsigned char)s[p]))
                    v=v*16+(isdigit((unsigned char)s[p])?s[p]-'0':toupper((unsigned char)s[p])-'A'+10),p++;
            } else if(s[p]=='0'&&isdigit((unsigned char)s[p+1])){
                p++; while(s[p]>='0'&&s[p]<='7') v=v*8+(s[p++]-'0');
            } else while(isdigit((unsigned char)s[p])) v=v*10+(s[p++]-'0');
            if(neg)v=-v;
            tk.t=TK_NUM; tk.v=(int32_t)(((int32_t)v)&0xffff);
            if(tk.v<0)tk.v=(tk.v+SIZE)&0xffff;
            l.p=p; out[n++]=tk; continue;}
        /* label def */
        if(s[p]==':'){
            p++; int loc=(s[p]=='.'); if(loc)p++;
            int st=p; while(isalnum((unsigned char)s[p])||s[p]=='_')p++;
            char buf[MAX_STR]; int len=p-st; strncpy(buf,s+st,len);buf[len]=0;
            char ubuf[MAX_STR]; strupr2(ubuf,buf);
            tk.t=TK_LABEL;
            if(loc) snprintf(tk.s,MAX_STR,"%s.%s",l.pfx,ubuf);
            else{ strncpy(tk.s,ubuf,MAX_STR-1); strncpy(l.pfx,ubuf,MAX_STR-1);}
            l.p=p; out[n++]=tk; continue;}
        /* identifier */
        if(isalpha((unsigned char)s[p])||s[p]=='_'||s[p]=='.'){
            int loc=(s[p]=='.'), st=p; if(loc)p++;
            while(isalnum((unsigned char)s[p])||s[p]=='_')p++;
            char raw[MAX_STR]; int len=p-st; strncpy(raw,s+st,len);raw[len]=0;
            char up[MAX_STR]; strupr2(up,raw);
            tk.t=TK_ID;
            if(loc) snprintf(tk.s,MAX_STR,"%s%s",l.pfx,raw);
            else strncpy(tk.s,up,MAX_STR-1);
            l.p=p; out[n++]=tk; continue;}
        {char m[32];snprintf(m,32,"unexpected '%c'",s[p]);lerr(&l,m);}
    }
    return n;
}

/* ─── token stream ───────────────────────────────────────── */
typedef struct{ Tok *t; int n,p; } TS;
static Tok peek(TS *ts){ return ts->t[ts->p]; }
static Tok cons(TS *ts){ return ts->t[ts->p++]; }
static int  ateof(TS *ts){ return ts->t[ts->p].t==TK_EOF; }
static void perr(TS *ts,const char *m){
    fprintf(stderr,"Parse error: %s (near '%s')\n",m,peek(ts).s);exit(1);}

static int is_id(TS *ts,const char *name){
    return peek(ts).t==TK_ID && !strcmp(peek(ts).s,name);}

/* ─── label table ────────────────────────────────────────── */
typedef struct{ char n[MAX_STR]; uint16_t off; } Lab;
typedef struct{ Lab *e; int cnt; } LT;
static void lt_init(LT *l){ l->e=malloc(MAX_LABELS*sizeof(Lab)); l->cnt=0; }
static void lt_free(LT *l){ free(l->e); }
static void lt_add(LT *l,const char *n,uint16_t o){
    strncpy(l->e[l->cnt].n,n,MAX_STR-1); l->e[l->cnt].off=o; l->cnt++;}
static int lt_get(const LT *l,const char *n,uint16_t *o){
    for(int i=0;i<l->cnt;i++) if(!strcmp(l->e[i].n,n)){*o=l->e[i].off;return 1;}
    return 0;}

/* ─── operand ────────────────────────────────────────────── */
typedef enum{WK_NONE,WK_NUM,WK_SYM}WK;
typedef struct{int val;WK wk;uint16_t wn;char ws[MAX_STR];int is_dst;}Op;
static int op_hw(const Op *o){return o->wk!=WK_NONE;}
static uint16_t op_res(const Op *o,const LT *lt){
    if(o->wk==WK_NUM)return o->wn;
    uint16_t v=0;
    if(!lt_get(lt,o->ws,&v)){fprintf(stderr,"Undefined: \"%s\"\n",o->ws);exit(1);}
    return v;}

/* ─── instruction ────────────────────────────────────────── */
typedef enum{IK_BASIC,IK_SPEC,IK_CMD,IK_DATA,IK_RESERVE,IK_LABEL}IK;

/* Data words stored separately to keep Instruction small */
typedef struct{
    IK     kind;
    int    op;
    Op     dst,src;
    uint16_t val;         /* encoded instruction word */
    /* data / reserve */
    int    data_idx;      /* index into global data pool */
    int    data_len;
    int    reserve_sz;
    char   label_name[MAX_STR];
    int    sz,off,cond;
} Instr;

/* Global data pool: words + symbol flags */
#define DATA_POOL 524288
static uint16_t g_data[DATA_POOL];
static char     g_dsym[DATA_POOL][MAX_STR]; /* "" = numeric */
static int      g_dpool=0;

/* ─── program ────────────────────────────────────────────── */
typedef struct{ Instr *ins; int cnt; LT labels; int total; } Prog;
static Prog *prog_new(void){
    Prog *p=malloc(sizeof(Prog));
    p->ins=malloc(MAX_INSTRS*sizeof(Instr));
    p->cnt=0; p->total=0; lt_init(&p->labels); return p;}
static void prog_free(Prog *p){ free(p->ins); lt_free(&p->labels); free(p); }

/* ─── pretty helpers ─────────────────────────────────────── */
static void pv(char *b,int l,uint16_t x){
    if(x<=0xff)snprintf(b,l,"%d",x);else snprintf(b,l,"0x%04x",x);}
static void pop(char *b,int bz,const Op *o){
    int x=o->val; char w[MAX_STR]="";
    if(o->wk==WK_NUM) pv(w,MAX_STR,o->wn);
    else if(o->wk==WK_SYM) strncpy(w,o->ws,MAX_STR-1);
    const char *rn;
    if((rn=rlu(REGS,x))){snprintf(b,bz,"%s",rn);return;}
    if(x>=8&&x<=15&&(rn=rlu(REGS,x-8))){snprintf(b,bz,"[%s]",rn);return;}
    if(x>=0x10&&x<=0x17&&(rn=rlu(REGS,x-0x10))){snprintf(b,bz,"[%s + %s]",rn,w);return;}
    const OE *cc=o->is_dst?DSTC:SRCC;
    if((rn=rlu(cc,x))){snprintf(b,bz,"%s",rn);return;}
    if(x==0x1a){snprintf(b,bz,"PICK %s",w);return;}
    if(x==0x1e){snprintf(b,bz,"[%s]",w);return;}
    if(x==0x1f){snprintf(b,bz,"%s",w);return;}
    if(x==0x20){snprintf(b,bz,"0xffff");return;}
    if(x>=0x21){pv(b,bz,(uint16_t)(x-0x21));return;}
    snprintf(b,bz,"???");}

/* ─── parse literal (number or label ref) ────────────────── */
static void plit(TS *ts, Op *o){
    Tok t=peek(ts);
    if(t.t==TK_NUM){cons(ts);o->wk=WK_NUM;o->wn=(uint16_t)t.v;}
    else if(t.t==TK_ID){cons(ts);o->wk=WK_SYM;strncpy(o->ws,t.s,MAX_STR-1);}
    else if(t.t==TK_STR){
        cons(ts); /* eat sentinel */
        /* use first char value as literal */
        Tok c=peek(ts);
        if(c.t==TK_NUM){cons(ts);o->wk=WK_NUM;o->wn=(uint16_t)c.v;}
        else{o->wk=WK_NUM;o->wn=0;}
    } else perr(ts,"expected literal");}

/* ─── parse dst operand ──────────────────────────────────── */
static void pdst(TS *ts,Op *o){
    memset(o,0,sizeof(*o)); o->is_dst=1;
    if(is_id(ts,"PICK")){cons(ts);o->val=0x1a;plit(ts,o);return;}
    if(peek(ts).t==TK_LB){
        cons(ts);
        /* [--SP] PUSH */
        if(peek(ts).t==TK_DEC){
            int sv=ts->p; cons(ts);
            if(is_id(ts,"SP")){cons(ts);
                if(peek(ts).t==TK_RB){cons(ts);o->val=0x18;return;}}
            ts->p=sv;}
        /* [SP] PEEK or [SP+lit] PICK */
        if(is_id(ts,"SP")){
            int sv=ts->p; cons(ts);
            if(peek(ts).t==TK_RB){cons(ts);o->val=0x19;return;}
            if(peek(ts).t==TK_PLUS){cons(ts);o->val=0x1a;plit(ts,o);
                if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}
            ts->p=sv;}
        /* [reg] [reg+lit] */
        if(peek(ts).t==TK_ID&&lu(REGS,peek(ts).s)>=0){
            Tok r=cons(ts); int reg=lu(REGS,r.s);
            if(peek(ts).t==TK_RB){cons(ts);o->val=reg+8;return;}
            if(peek(ts).t==TK_PLUS){cons(ts);o->val=reg+0x10;plit(ts,o);
                if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}
            perr(ts,"expected ] or +");}
        /* [lit] or [lit+reg] */
        Op tmp;memset(&tmp,0,sizeof(tmp));plit(ts,&tmp);
        if(peek(ts).t==TK_PLUS){cons(ts);
            if(peek(ts).t==TK_ID&&lu(REGS,peek(ts).s)>=0){
                Tok r=cons(ts); int reg=lu(REGS,r.s);
                o->val=reg+0x10;o->wk=tmp.wk;o->wn=tmp.wn;
                strncpy(o->ws,tmp.ws,MAX_STR-1);
                if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}}
        o->val=0x1e;o->wk=tmp.wk;o->wn=tmp.wn;strncpy(o->ws,tmp.ws,MAX_STR-1);
        if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}
    /* register */
    if(peek(ts).t==TK_ID&&lu(REGS,peek(ts).s)>=0){o->val=lu(REGS,cons(ts).s);return;}
    /* dst code */
    if(peek(ts).t==TK_ID&&lu(DSTC,peek(ts).s)>=0){o->val=lu(DSTC,cons(ts).s);return;}
    o->val=0x1f;plit(ts,o);}

/* ─── parse src operand ──────────────────────────────────── */
static void psrc(TS *ts,Op *o){
    memset(o,0,sizeof(*o));
    if(is_id(ts,"PICK")){cons(ts);o->val=0x1a;plit(ts,o);return;}
    if(peek(ts).t==TK_LB){
        cons(ts);
        /* [SP++] POP */
        if(is_id(ts,"SP")){
            int sv=ts->p; cons(ts);
            if(peek(ts).t==TK_INC){cons(ts);
                if(peek(ts).t==TK_RB){cons(ts);o->val=0x18;return;}}
            ts->p=sv; cons(ts); /* re-consume SP */
            if(peek(ts).t==TK_RB){cons(ts);o->val=0x19;return;}
            if(peek(ts).t==TK_PLUS){cons(ts);o->val=0x1a;plit(ts,o);
                if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}
            ts->p=sv;}
        if(peek(ts).t==TK_ID&&lu(REGS,peek(ts).s)>=0){
            Tok r=cons(ts); int reg=lu(REGS,r.s);
            if(peek(ts).t==TK_RB){cons(ts);o->val=reg+8;return;}
            if(peek(ts).t==TK_PLUS){cons(ts);o->val=reg+0x10;plit(ts,o);
                if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}
            perr(ts,"expected ] or +");}
        Op tmp;memset(&tmp,0,sizeof(tmp));plit(ts,&tmp);
        if(peek(ts).t==TK_PLUS){cons(ts);
            if(peek(ts).t==TK_ID&&lu(REGS,peek(ts).s)>=0){
                Tok r=cons(ts); int reg=lu(REGS,r.s);
                o->val=reg+0x10;o->wk=tmp.wk;o->wn=tmp.wn;
                strncpy(o->ws,tmp.ws,MAX_STR-1);
                if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}}
        o->val=0x1e;o->wk=tmp.wk;o->wn=tmp.wn;strncpy(o->ws,tmp.ws,MAX_STR-1);
        if(peek(ts).t==TK_RB){cons(ts);return;}perr(ts,"] expected");}
    if(peek(ts).t==TK_ID&&lu(REGS,peek(ts).s)>=0){o->val=lu(REGS,cons(ts).s);return;}
    if(peek(ts).t==TK_ID&&lu(SRCC,peek(ts).s)>=0){o->val=lu(SRCC,cons(ts).s);return;}
    /* literal with small-constant encoding */
    Op tmp;memset(&tmp,0,sizeof(tmp));plit(ts,&tmp);
    if(tmp.wk==WK_NUM){
        uint16_t v=tmp.wn;
        if(v==0xffff){o->val=0x20;return;}
        if(v<=0x1e){o->val=0x21+v;return;}}
    o->val=0x1f;o->wk=tmp.wk;o->wn=tmp.wn;strncpy(o->ws,tmp.ws,MAX_STR-1);}

/* ─── main parse ─────────────────────────────────────────── */
static void prog_parse(Prog *prog, const char *src){
    Tok *toks=malloc(MAX_TOKENS*sizeof(Tok));
    tokenise(src,toks,MAX_TOKENS);
    TS ts; ts.t=toks; ts.n=MAX_TOKENS; ts.p=0;
    while(!ateof(&ts)){
        Tok t=peek(&ts);
        Instr *in=&prog->ins[prog->cnt];
        memset(in,0,sizeof(*in));
        in->off=prog->total;
        /* label */
        if(t.t==TK_LABEL){
            cons(&ts); in->kind=IK_LABEL;
            strncpy(in->label_name,t.s,MAX_STR-1);
            lt_add(&prog->labels,t.s,(uint16_t)prog->total);
            prog->cnt++;
            if(peek(&ts).t==TK_AT){cons(&ts);
                Tok n=cons(&ts);
                prog->labels.e[prog->labels.cnt-1].off=(uint16_t)n.v;}
            continue;}
        if(t.t!=TK_ID){perr(&ts,"expected instruction");}
        const char *id=t.s;
        /* DAT */
        if(!strcmp(id,"DAT")){
            cons(&ts); in->kind=IK_DATA;
            in->data_idx=g_dpool; in->data_len=0;
            while(1){
                Tok dt=peek(&ts);
                if(dt.t==TK_NUM){
                    cons(&ts);
                    g_data[g_dpool]=(uint16_t)dt.v;
                    g_dsym[g_dpool][0]=0; g_dpool++; in->data_len++;}
                else if(dt.t==TK_STR){
                    cons(&ts); /* eat sentinel, consume following NUM chars */
                    while(peek(&ts).t==TK_NUM){
                        Tok c=cons(&ts);
                        g_data[g_dpool]=(uint16_t)c.v;
                        g_dsym[g_dpool][0]=0; g_dpool++; in->data_len++;}}
                else if(dt.t==TK_ID){
                    // NUEVO: Si la palabra es otro DAT o una instrucción, cortamos el bucle
                    if (!strcmp(dt.s, "DAT") || lu(BOP, dt.s) >= 0 || lu(SOP, dt.s) >= 0) break;

                    cons(&ts);
                    g_data[g_dpool]=0;
                    strncpy(g_dsym[g_dpool],dt.s,MAX_STR-1); g_dpool++; in->data_len++;}
                else break;}
            in->sz=in->data_len; prog->total+=in->sz; prog->cnt++; continue;}
        /* RESERVE */
        if(!strcmp(id,"RESERVE")){
            cons(&ts); Tok n=cons(&ts);
            in->kind=IK_RESERVE; in->reserve_sz=n.v;
            in->sz=n.v; prog->total+=in->sz; prog->cnt++; continue;}
        /* command */
        {int c=lu(COP,id);if(c>=0){
            cons(&ts); in->kind=IK_CMD; in->val=(uint16_t)c;
            in->sz=1; prog->total++; prog->cnt++; continue;}}
        /* special */
        {int c=lu(SOP,id);if(c>=0){
            cons(&ts); in->kind=IK_SPEC; in->op=c;
            psrc(&ts,&in->src);
            in->val=(uint16_t)(((c&0x1f)<<5)|((in->src.val&0x3f)<<10));
            in->sz=1+(op_hw(&in->src)?1:0);
            prog->total+=in->sz; prog->cnt++; continue;}}
        /* basic */
        {int c=lu(BOP,id);if(c>=0){
            cons(&ts); in->kind=IK_BASIC; in->op=c;
            pdst(&ts,&in->dst); psrc(&ts,&in->src);
            in->val=(uint16_t)(c|((in->dst.val&0x1f)<<5)|((in->src.val&0x3f)<<10));
            in->sz=1+(op_hw(&in->dst)?1:0)+(op_hw(&in->src)?1:0);
            in->cond=(c>=0x10&&c<=0x17);
            prog->total+=in->sz; prog->cnt++; continue;}}
        perr(&ts,"unknown instruction");}
    free(toks);}

/* ─── assemble ───────────────────────────────────────────── */
static int prog_asm(const Prog *p, uint16_t *out){
    int n=0;
    for(int i=0;i<p->cnt;i++){
        const Instr *in=&p->ins[i];
        switch(in->kind){
        case IK_LABEL: break;
        case IK_RESERVE: for(int j=0;j<in->reserve_sz;j++)out[n++]=0; break;
        case IK_DATA:
            for(int j=0;j<in->data_len;j++){
                int idx=in->data_idx+j;
                if(g_dsym[idx][0]){
                    uint16_t v=0;
                    if(!lt_get(&p->labels,g_dsym[idx],&v))
                        {fprintf(stderr,"Undefined: \"%s\"\n",g_dsym[idx]);exit(1);}
                    out[n++]=v;
                } else out[n++]=g_data[idx];}
            break;
        case IK_CMD: out[n++]=in->val; break;
        case IK_SPEC:
            out[n++]=in->val;
            if(op_hw(&in->src))out[n++]=op_res(&in->src,&p->labels); break;
        case IK_BASIC:
            out[n++]=in->val;
            if(op_hw(&in->src))out[n++]=op_res(&in->src,&p->labels);
            if(op_hw(&in->dst))out[n++]=op_res(&in->dst,&p->labels); break;}}
    return n;}

/* ─── pretty print ───────────────────────────────────────── */
static void prog_pretty(Prog *p){
    uint16_t *words=malloc(MAX_WORDS*sizeof(uint16_t));
    prog_asm(p,words);
    int skip=0;
    for(int i=0;i<p->cnt;i++){
        const Instr *in=&p->ins[i];
        char line[512]=""; int pad=0;
        switch(in->kind){
        case IK_LABEL: snprintf(line,sizeof(line),":%s",in->label_name);pad=0;break;
        case IK_RESERVE:{char v[32];pv(v,32,(uint16_t)in->reserve_sz);
            snprintf(line,sizeof(line),"RESERVE %s",v);pad=skip?4:2;break;}
        case IK_DATA:{
            char tmp[2048]="DAT ";
            for(int j=0;j<in->data_len;j++){
                char v[32];pv(v,32,g_data[in->data_idx+j]);
                if(j)strncat(tmp,", ",sizeof(tmp)-strlen(tmp)-1);
                strncat(tmp,g_dsym[in->data_idx+j][0]?g_dsym[in->data_idx+j]:v,
                    sizeof(tmp)-strlen(tmp)-1);}
            strncpy(line,tmp,sizeof(line)-1);pad=skip?4:2;break;}
        case IK_CMD:
            snprintf(line,sizeof(line),"%s",rlu(COP,in->val));pad=skip?4:2;break;
        case IK_SPEC:{char sb[64];pop(sb,sizeof(sb),&in->src);
            snprintf(line,sizeof(line),"%s %s",rlu(SOP,in->op),sb);pad=skip?4:2;break;}
        case IK_BASIC:{char db[64],sb[64];
            pop(db,sizeof(db),&in->dst);pop(sb,sizeof(sb),&in->src);
            snprintf(line,sizeof(line),"%s %s, %s",rlu(BOP,in->op),db,sb);pad=skip?4:2;break;}}
        for(int j=0;j<pad;j++)putchar(' ');
        printf("%s",line);
        if(in->kind!=IK_LABEL&&in->kind!=IK_DATA&&in->sz>0){
            int col=pad+(int)strlen(line);
            for(int j=col;j<32;j++)putchar(' ');
            printf("; ");
            for(int j=0;j<in->sz;j++){if(j)putchar(' ');printf("%04x",words[in->off+j]);}}
        putchar('\n');
        skip=in->cond;}
    free(words);}

/* ─── disassembler ───────────────────────────────────────── */
static int unw(int v){return(v>=0x10&&v<=0x17)||v==0x1a||v==0x1e||v==0x1f;}
static void mop(Op *o,int val,uint16_t w,int hw,int id){
    memset(o,0,sizeof(*o));o->val=val;o->is_dst=id;
    if(hw){o->wk=WK_NUM;o->wn=w;}}
static void disasm(const uint16_t *ws,int n){
    int p=0;
    while(p<n){
        uint16_t w=ws[p++];
        int op=w&0x1f,dst=(w>>5)&0x1f,src=(w>>10)&0x3f;
        if(op&&rlu(BOP,op)){
            uint16_t dw=0,sw=0;
            int dh=unw(dst),sh=unw(src);
            if(sh&&p<n)sw=ws[p++];
            if(dh&&p<n)dw=ws[p++];
            Op dop,sop; mop(&dop,dst,dw,dh,1); mop(&sop,src,sw,sh,0);
            char db[64],sb[64];pop(db,64,&dop);pop(sb,64,&sop);
            printf("  %s %s, %s\n",rlu(BOP,op),db,sb);
        } else if(!op&&rlu(SOP,dst)){
            uint16_t sw=0; int sh=unw(src);
            if(sh&&p<n)sw=ws[p++];
            Op sop; mop(&sop,src,sw,sh,0);
            char sb[64];pop(sb,64,&sop);
            printf("  %s %s\n",rlu(SOP,dst),sb);
        } else printf("  DAT 0x%04x\n",w);}}

/* ─── file I/O ───────────────────────────────────────────── */
static char *rfile(const char *p){
    FILE *f=fopen(p,"r");if(!f){perror(p);exit(1);}
    fseek(f,0,SEEK_END);long l=ftell(f);rewind(f);
    char *b=malloc(l+2);size_t r=fread(b,1,l,f);b[r]=0;fclose(f);return b;}
static void wbin(const char *p,const uint16_t *w,int n){
    // Abrimos en modo texto ("w") en lugar de modo binario ("wb")
    FILE *f = fopen(p, "w");
    if(!f) { perror(p); exit(1); }

    for(int i = 0; i < n; i++) {
        // Escribe el número en texto hexadecimal de 4 dígitos (ej: "1e00 ")
        fprintf(f, "%04x ", w[i]);

        // Cada 8 palabras mete un salto de línea para que el archivo sea bonito y limpio
        if ((i + 1) % 8 == 0) {
            fprintf(f, "\n");
        }
    }
    fclose(f);
}
static int rbin(const char *p,uint16_t *o,int m){
    FILE *f=fopen(p,"rb");if(!f){perror(p);exit(1);}
    int n=0;uint8_t b[2];
    while(n<m&&fread(b,1,2,f)==2)o[n++]=((uint16_t)b[0]<<8)|b[1];
    fclose(f);return n;}

/* ─── main ───────────────────────────────────────────────── */
static void usage(const char *p){
    fprintf(stderr,"Usage:\n  %s assemble <in.asm> [out.bin]\n  %s disasm <in.bin>\n  %s pretty <in.asm>\n",p,p,p);
    exit(1);}

int main(int argc,char **argv){
    if(argc<3)usage(argv[0]);
    const char *cmd=argv[1],*in=argv[2];
    if(!strcmp(cmd,"assemble")){
        char *src=rfile(in);
        Prog *p=prog_new(); prog_parse(p,src); free(src);
        uint16_t *w=malloc(MAX_WORDS*sizeof(uint16_t));
        int n=prog_asm(p,w);
        const char *out=argc>=4?argv[3]:"output.bin";
        wbin(out,w,n);
        printf("Assembled %d words → %s\n",n,out);
        free(w);prog_free(p);
    } else if(!strcmp(cmd,"disasm")){
        uint16_t *w=malloc(MAX_WORDS*sizeof(uint16_t));
        int n=rbin(in,w,MAX_WORDS);
        printf("; Disassembly of %s (%d words)\n",in,n);
        disasm(w,n);free(w);
    } else if(!strcmp(cmd,"pretty")){
        char *src=rfile(in);
        Prog *p=prog_new(); prog_parse(p,src); free(src);
        prog_pretty(p); prog_free(p);
    } else usage(argv[0]);
    return 0;}