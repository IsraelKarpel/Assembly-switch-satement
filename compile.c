#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

//rdi *p1      rsi *p2             rdx  action      rax result

int main() {
    int double_operatpr_flag = false;
    int shift_by_register = false;
    FILE *fpw = fopen("switch.s", "w");
    bool flag = false;
    int cap = 0;
    int i, j, temp, num;
    int *cases_numbers = (int *) malloc(sizeof(int));
    char buffer[100];
    FILE *fpr = fopen("switch.c", "r");
    if (fpr == NULL) {
        printf("Error! open file");
    }
    //get the cases numbers
    char *line = fgets(buffer, 100, fpr);
    while (line != NULL) {
        char *word = strtok(line, " ");
        //get the number cases into the cases array
        if (!strcmp(word, "case")) {
            char *cases_num = strtok(NULL, ":");
            cap++;
            cases_numbers = realloc(cases_numbers, cap * sizeof(int));
            cases_numbers[cap - 1] = atoi(cases_num);
        }
        line = fgets(buffer, 100, fpr);
    }
    //sort the array
    int cases_numbers2[cap];
    for (i = 0; i < cap; i++) {
        num = 0;
        temp = cases_numbers[i];
        for (j = 0; j < cap; j++) {
            if (cases_numbers[j] < temp) {
                temp = cases_numbers[j];
                num = j;
            }
        }
        cases_numbers2[i] = temp;
        cases_numbers[num] = 999999;
    }
    //reduce by the lowest value
    int cases_numbers2_decrease[cap];
    for (i = 0; i < cap; i++) {
        cases_numbers2_decrease[i] = cases_numbers2[i] - cases_numbers2[0];
    }
    fputs("\t.section .text\n\t.global switch2\nswitch2:\n", fpw);
    //rcx result 0
    fputs( "movq $0,%rax\n", fpw);
    fprintf(fpw, "subq $%d, %%rdx\n", cases_numbers2[0]);
    fprintf(fpw, "cmpq $%d, %%rdx\n", cases_numbers2_decrease[cap - 1]);
    fputs("ja .L1\n", fpw);
    //fprintf(fpw, "ja .L%d\n", cases_numbers2_decrease[cap-1]);
    fputs("jmp *.L2(,%rdx,8)\n", fpw);


    //for (i = 0; i < cap; i++) {
        fpr = fopen("switch.c", "r");
        line = fgets(buffer, 100, fpr);
        while (line != NULL) {
            char *word = strtok(line, " ");
            //get the number cases into the cases array
            if (!strcmp(word, "case")) {
                char *cases_num = strtok(NULL, ":");
                for (i = 0; i < cap; i++) {
                    if (atoi(cases_num) == cases_numbers2[i]) {
                        fprintf(fpw, ".L%d:\n", i + 4);
                        line = fgets(buffer, 100, fpr);
                        while (true) {
                            char *p = strstr(line, "case");
                            if (p != NULL) {
                                break;
                            }
                            char *left = strtok(line, " ");
                            char *op = strtok(NULL, " ");
                            char *right = strtok(NULL, ";");
                            if (!strcmp(left,"case")) {
                                break;
                            }
                            //checks for double oger
                            if ((!strcmp(right,"*p1")) && ((!strcmp(left,"*p2")) || (!strcmp(left,"*p1")))) {
                                fputs("movq (%rdi), %rcx\n", fpw);
                                double_operatpr_flag = true;
                            }
                            if ((!strcmp(right,"*p2")) && ((!strcmp(left,"*p2")) || (!strcmp(left,"*p1")))) {
                                fputs("movq (%rsi), %rcx\n", fpw);
                                double_operatpr_flag = true;
                            }
                            //checking what is the operator
                            if (!strcmp(op, "=")) {
                                fputs("movq ", fpw);
                            }
                            if (!strcmp(op, "+=")) {
                                fputs("addq ", fpw);
                            }
                            if (!strcmp(op, "-=")) {
                                fputs("subq ", fpw);
                            }
                            if (!strcmp(op, "*=")) {
                                fputs("imulq ", fpw);
                            }
                            if (!strcmp(op, "<<=")) {
                                if ((!strcmp(right, "*p1")) || (!strcmp(right, "*p2"))) {
                                    shift_by_register = true;
                                    fputs("movq ", fpw);
                                } else {
                                    fputs("salq ", fpw);
                                }
                            }
                            if (!strcmp(op, ">>=")) {
                                if ((!strcmp(right, "*p1")) || (!strcmp(right, "*p2"))) {
                                    shift_by_register = true;
                                    fputs("movq ", fpw);
                                } else {
                                    fputs("sarq ", fpw);
                                }
                            }

                            if (double_operatpr_flag == true) {
                                fputs ("%rcx, ", fpw);
                                double_operatpr_flag = false;
                            } else {
                                //checks right
                                if (!strcmp(right, "*p1")) {
                                    fputs("(%rdi), ", fpw);
                                } else if (!strcmp(right, "*p2")) {
                                    fputs("(%rsi), ", fpw);
                                } else if (!strcmp(right, "result")) {
                                    fputs("%rax, ", fpw);
                                } else {
                                    fprintf(fpw, "$%s, ", right);
                                }
                            }
                            if (shift_by_register == true) {
                                shift_by_register = false;
                                fputs("%rcx\n",fpw);
                                if (!strcmp(op,"<<=")) {
                                    fputs("salq %cl, ",fpw);
                                }
                                if (!strcmp(op,">>=")) {
                                    fputs("sarq %cl, ",fpw);
                                }
                            }
                            //checks left:
                            if (!strcmp(left, "*p1")) {
                                    fputs("(%rdi)\n", fpw);
                            }
                            if (!strcmp(left, "*p2")) {
                                    fputs("(%rsi)\n", fpw);
                            }
                            if (!strcmp(left, "result")) {
                                fputs("%rax\n", fpw);
                            }
                            line = fgets(buffer, 100, fpr);
                            if (!(strcmp(line, " break;\n"))) {
                                fputs("jmp .L3\n", fpw);
                                break;
                            }
                            if (strstr(line, "case:")) {
                                break;
                            }
                        }
                    }
                }
            } else {
                line = fgets(buffer, 100, fpr);
            }
        }
    //}
    //checks the default
    fpr = fopen("switch.c", "r");
    line = fgets(buffer, 100, fpr);
    while (line != NULL) {
        char *word = strtok(line, " ");
        if (!(strcmp(word, "default:\n"))) {
            fputs(".L1:\n", fpw);
            line = fgets(buffer, 100, fpr);
            while ((strcmp(line, "}\n"))) {
                char *left = strtok(line, " ");
                char *op = strtok(NULL, " ");
                char *right = strtok(NULL, ";");
                if ((!strcmp(right,"*p1")) && ((!strcmp(left,"*p2")) || (!strcmp(left,"*p1")))) {
                    fputs("movq (%rdi), %rcx\n", fpw);
                    double_operatpr_flag = true;
                }
                if ((!strcmp(right,"*p2")) && ((!strcmp(left,"*p2")) || (!strcmp(left,"*p1")))) {
                    fputs("movq (%rsi), %rcx\n", fpw);
                    double_operatpr_flag = true;
                }
                //checking what is the operator
                if (!strcmp(op, "=")) {
                    fputs("movq ", fpw);
                }
                if (!strcmp(op, "+=")) {
                    fputs("addq ", fpw);
                }
                if (!strcmp(op, "-=")) {
                    fputs("subq ", fpw);
                }
                if (!strcmp(op, "*=")) {
                    fputs("imulq", fpw);
                }
                if (!strcmp(op, "<<=")) {
                    if ((!strcmp(right, "*p1")) || (!strcmp(right, "*p2"))) {
                        shift_by_register = true;
                        fputs("movq ", fpw);
                    } else {
                        fputs("salq ", fpw);
                    }
                }
                if (!strcmp(op, ">>=")) {
                    if ((!strcmp(right, "*p1")) || (!strcmp(right, "*p2"))) {
                        shift_by_register = true;
                        fputs("movq ", fpw);
                    } else {
                        fputs("sarq ", fpw);
                    }
                }
                if (double_operatpr_flag == true) {
                    fputs ("%rcx, ", fpw);
                    double_operatpr_flag = false;
                } else {
                    //checks right
                    if (!strcmp(right, "*p1")) {
                        fputs("(%rdi), ", fpw);
                    } else if (!strcmp(right, "*p2")) {
                        fputs("(%rsi), ", fpw);
                    } else if (!strcmp(right, "result")) {
                        fputs("%rax, ", fpw);
                    } else {
                        fprintf(fpw, "$%s, ", right);
                    }
                }
                if (shift_by_register == true) {
                    shift_by_register = false;
                    fputs("%rcx\n",fpw);
                    if (!strcmp(op,"<<=")) {
                        fputs("salq %cl, ",fpw);
                    }
                    if (!strcmp(op,">>=")) {
                        fputs("sarq %cl, ",fpw);
                    }
                }
                //checks left:
                if (!strcmp(left, "*p1")) {
                    fputs("(%rdi)\n", fpw);
                }
                if (!strcmp(left, "*p2")) {
                    fputs("(%rsi)\n", fpw);
                }
                if (!strcmp(left, "result")) {
                    fputs("%rax\n", fpw);
                }
                line = fgets(buffer, 100, fpr);
            }
        }
            line = fgets(buffer, 100, fpr);
    }

fputs(".L3:\nret\n",fpw);
//rdi rsi rdx



    //table jump rax
    //L1 default L2 tablemane L3 break go to done
    int num_table = 4;
    fputs("\t.section\t.rodata\n", fpw);
    fputs("\t.align 8\n", fpw);
    fputs(".L2:\n", fpw);
    for (i = 0; i < cases_numbers2_decrease[cap - 1] + 1; i++) {
        for (j = 0; j < cases_numbers2_decrease[cap - 1]; j++) {
            if (i == cases_numbers2_decrease[j]) {
                flag = true;
            }
        }
        if (flag == false) {
            fputs(".quad .L1\n", fpw);
        } else {
            fprintf(fpw, ".quad .L%d\n", num_table);
            num_table++;
        }
        flag = false;
    }

    free(cases_numbers);
    fclose(fpr);
    fclose(fpw);
    return 0;
}