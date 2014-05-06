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
�ݹ��㷨
--------
�����ǰ�ڵ���Ҷ�ӽڵ㣺
   ���Ƶ�ǰ�ڵ㣬���أ�
������ӽڵ㣺
   ���������ӽڵ㣻
   �����Լ���
   ���Ƽ�ͷ��

*/
void exNode
    (   nodeType *p,
        int c, int l,        /* start column and line of node */
        int *ce, int *cm     /* resulting end column and mid of node: ����н�β������+1,������м����� */
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
    *ce = c + w;      //���ĵ�һ�����У�  _ab [<-cm] c[ce]
    *cm = c + w / 2;  //�м���

    /* node is leaf */
    if (!p || p->type == typeCon || p->type == typeId || p->opr.nops == 0) {
        graphDrawBox (s, cbar, l);
        return;
    }

    /* node has children */
    cs = c;  //��ʼ��
    for (k = 0; k < p->opr.nops; k++) { //���Ƹ��ӽڵ�
        exNode (p->opr.op[k], cs, l+h+eps, &che, &chm); //�м������ڷָ���eps��=3����
        cs = che; // ����
    }

    /* total node width */
    if (w < che - c) {   //�ӽڵ��ܿ�ȴ��� ��ǰ�ڵ�Ŀ�ȣ���Ҫ������ǰ�Ľڵ����ʼ�����
        cbar += (che - c - w) / 2;
        *ce = che;
        *cm = (c + che) / 2;
    }

    /* draw node */
    graphDrawBox (s, cbar, l);  //�����ǰ�ڵ�

    /* draw arrows (not optimal: children are drawn a second time) */
    cs = c;
    for (k = 0; k < p->opr.nops; k++) {
        exNode (p->opr.op[k], cs, l+h+eps, &che, &chm);//Ϊ�˵õ�che��chm
        graphDrawArrow (*cm, l+h, chm, l+h+eps-1);
        cs = che;
    }
}

/* interface for drawing */

#define lmax 200
#define cmax 200

//���ַ������ʾһ����λͼ��
char graph[lmax][cmax]; /* array for ASCII-Graphic */
int graphNumber = 0;

//����(l,c)�����Ƿ��ھ���Χ��
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

//�ÿո��ʼ�������ء��ַ�����
void graphInit (void) {
    int i, j;
    for (i = 0; i < lmax; i++) {
        for (j = 0; j < cmax; j++) {
            graph[i][j] = ' ';
        }
    }
}

//��ӡ��λͼ��
void graphFinish() {
    int i, j;
    //�����ַ����� --> ȷ��ÿ���ַ�����0��β
    for (i = 0; i < lmax; i++) {
        for (j = cmax-1; j > 0 && graph[i][j] == ' '; j--);
        graph[i][cmax-1] = 0;
        if (j < cmax-1) graph[i][j+1] = 0;
        if (graph[i][j] == ' ') graph[i][j] = 0;
    }
    for (i = lmax-1; i > 0 && graph[i][0] == 0; i--); //�ҵ������ݵ����һ��
    printf ("\n\nGraph %d:\n", graphNumber++);
    for (j = 0; j <= i; j++) printf ("\n%s", graph[j]); //���д�ӡ�ַ���
    printf("\n");
}

//�����ַ���s�Ŀ�ȣ�������+1���͸߶ȣ�1����del��ǰ���ո��ַ���
void graphBox (char *s, int *w, int *h) {
    *w = strlen (s) + del;
    *h = 1;
}

//��s���Ƶ�����l�У�c�п�ʼ��λ�á�del��ǰ���ո��ַ���
void graphDrawBox (char *s, int c, int l) {
    int i;
    graphTest (l, c+strlen(s)-1+del); //���Ƿ�Խ��?
    for (i = 0; i < strlen (s); i++) {
        graph[l][c+i+del] = s[i];
    }
}
//��(c1, l1) (c2,l2)֮�����һ������
void graphDrawArrow (int c1, int l1, int c2, int l2) {
    int m;
    graphTest (l1, c1);
    graphTest (l2, c2);
    m = (l1 + l2) / 2; //�м��У���������ֱ��
    while (l1 != m) { graph[l1][c1] = '|'; if (l1 < l2) l1++; else l1--; } //����������
    while (c1 != c2) { graph[l1][c1] = '-'; if (c1 < c2) c1++; else c1--; } //���ƺ���
    while (l1 != l2) { graph[l1][c1] = '|'; if (l1 < l2) l1++; else l1--; } //����������
    graph[l1][c1] = '|';  //�������
}

