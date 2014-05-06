/* source code courtesy of Frank Thomas Braun */

/* calc3d.c: Generation of the graph of the syntax tree */

#include <stdio.h>
#include <string.h>

#include "calc3.h"
#include "y.tab.h"

int del = 1; /* distance of graph columns */
int eps = 3; /* distance of graph lines */

/* interface for drawing (can be replaced by "real" graphic using GD or other) */
void graphInit (void);
void graphFinish();
void graphBox (char *s, int *w, int *h);
void graphDrawBox (char *s, int c, int l);
void graphDrawArrow (int c1, int l1, int c2, int l2);

/* recursive drawing of the syntax tree */
void exNode (nodeType *p, int c, int l, int *ce, int *cm);

/*****************************************************************************/

/* main entry point of the manipulation of the syntax tree */
int ex (nodeType *p) {
    int rte, rtm;

    graphInit ();
    exNode (p, 0, 0, &rte, &rtm);
    graphFinish();
    return 0;
}

/*c----cm---ce---->                       drawing of leaf-nodes
 l leaf-info
 */

/*c---------------cm--------------ce----> drawing of non-leaf-nodes
 l            node-info
 *                |
 *    -------------     ...----
 *    |       |               |
 *    v       v               v
 * child1  child2  ...     child-n
 *        che     che             che
 *cs      cs      cs              cs
 *
 */

/*
递归算法
--------
如果当前节点是叶子节点：
   绘制当前节点，返回；
如果有子节点：
   绘制所有子节点；
   绘制自己；
   绘制箭头；

*/
void exNode
    (   nodeType *p,
        int c, int l,        /* start column and line of node */
        int *ce, int *cm     /* resulting end column and mid of node: 输出行结尾列坐标+1,输出行中间坐标 */
    )
{
    int w, h;           /* node width and height */
    char *s;            /* node text */
    int cbar;           /* "real" start column of node (centred above subnodes) */
    int k;              /* child number */
    int che, chm;       /* end column and mid of children */
    int cs;             /* start column of children */
    char word[20];      /* extended node text */


    //JYD
    //if (!p) return;

    strcpy (word, "???"); /* should never appear */
    s = word;
    if (!p)
    {
      s = "(null)";
      goto draw;
    }
    switch(p->type) {
        case typeCon: sprintf (word, "c(%d)", p->con.value); break;
        case typeId:  sprintf (word, "id(%c)", p->id.i + 'A'); break;
        case typeOpr:
            switch(p->opr.oper){
                case WHILE:     s = "while"; break;
                case IF:        s = "if";    break;
                case PRINT:     s = "print"; break;
                case ';':       s = "[;]";     break;
                case '=':       s = "[=]";     break;
                case UMINUS:    s = "[_]";     break;
                case '+':       s = "[+]";     break;
                case '-':       s = "[-]";     break;
                case '*':       s = "[*]";     break;
                case '/':       s = "[/]";     break;
                case '<':       s = "[<]";     break;
                case '>':       s = "[>]";     break;
                case GE:        s = "[>=]";    break;
                case LE:        s = "[<=]";    break;
                case NE:        s = "[!=]";    break;
                case EQ:        s = "[==]";    break;
            }
            break;
    }
draw:
    /* construct node text box */
    graphBox (s, &w, &h);
    cbar = c;
    *ce = c + w;      //随后的第一个空列：  _ab [<-cm] c[ce]
    *cm = c + w / 2;  //中间列

    /* node is leaf */
    if (!p || p->type == typeCon || p->type == typeId || p->opr.nops == 0) {
        graphDrawBox (s, cbar, l);
        return;
    }

    /* node has children */
    cs = c;  //起始列
    for (k = 0; k < p->opr.nops; k++) { //绘制各子节点
        exNode (p->opr.op[k], cs, l+h+eps, &che, &chm); //行加上用于分隔的eps（=3）行
        cs = che; // 新列
    }

    /* total node width */
    if (w < che - c) {   //子节点总宽度大于 当前节点的宽度，需要调整当前的节点的起始输出列
        cbar += (che - c - w) / 2;
        *ce = che;
        *cm = (c + che) / 2;
    }

    /* draw node */
    graphDrawBox (s, cbar, l);  //输出当前节点

    /* draw arrows (not optimal: children are drawn a second time) */
    cs = c;
    for (k = 0; k < p->opr.nops; k++) {
        exNode (p->opr.op[k], cs, l+h+eps, &che, &chm);//为了得到che和chm
        graphDrawArrow (*cm, l+h, chm, l+h+eps-1);
        cs = che;
    }
}

/* interface for drawing */

#define lmax 200
#define cmax 200

//用字符矩阵表示一个“位图”
char graph[lmax][cmax]; /* array for ASCII-Graphic */
int graphNumber = 0;

//测试(l,c)坐标是否在矩阵范围内
void graphTest (int l, int c)
{   int ok;
    ok = 1;
    if (l < 0) ok = 0;
    if (l >= lmax) ok = 0;
    if (c < 0) ok = 0;
    if (c >= cmax) ok = 0;
    if (ok) return;
    printf ("\n+++error: l=%d, c=%d not in drawing rectangle 0, 0 ... %d, %d",
        l, c, lmax, cmax);
    exit(1);
}

//用空格初始化“像素”字符矩阵
void graphInit (void) {
    int i, j;
    for (i = 0; i < lmax; i++) {
        for (j = 0; j < cmax; j++) {
            graph[i][j] = ' ';
        }
    }
}

//打印“位图”
void graphFinish() {
    int i, j;
    //修正字符矩阵 --> 确保每行字符串以0结尾
    for (i = 0; i < lmax; i++) {
        for (j = cmax-1; j > 0 && graph[i][j] == ' '; j--);
        graph[i][cmax-1] = 0;
        if (j < cmax-1) graph[i][j+1] = 0;
        if (graph[i][j] == ' ') graph[i][j] = 0;
    }
    for (i = lmax-1; i > 0 && graph[i][0] == 0; i--); //找到有内容的最后一行
    printf ("\n\nGraph %d:\n", graphNumber++);
    for (j = 0; j <= i; j++) printf ("\n%s", graph[j]); //逐行打印字符串
    printf("\n");
}

//返回字符串s的宽度（本身长度+1）和高度（1）。del是前导空格字符数
void graphBox (char *s, int *w, int *h) {
    *w = strlen (s) + del;
    *h = 1;
}

//把s绘制到：第l行，c列开始的位置。del是前导空格字符数
void graphDrawBox (char *s, int c, int l) {
    int i;
    graphTest (l, c+strlen(s)-1+del); //列是否越界?
    for (i = 0; i < strlen (s); i++) {
        graph[l][c+i+del] = s[i];
    }
}
//在(c1, l1) (c2,l2)之间绘制一个连线
void graphDrawArrow (int c1, int l1, int c2, int l2) {
    int m;
    graphTest (l1, c1);
    graphTest (l2, c2);
    m = (l1 + l2) / 2; //中间行：用来绘制直线
    while (l1 != m) { graph[l1][c1] = '|'; if (l1 < l2) l1++; else l1--; } //绘制上竖线
    while (c1 != c2) { graph[l1][c1] = '-'; if (c1 < c2) c1++; else c1--; } //绘制横线
    while (l1 != l2) { graph[l1][c1] = '|'; if (l1 < l2) l1++; else l1--; } //绘制下竖线
    graph[l1][c1] = '|';  //最后竖线
}

