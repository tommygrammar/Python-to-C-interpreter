#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to trim whitespace from a string
void trim(char* str) {
    char* end;
    while (isspace((unsigned char)*str)) str++;  // Trim leading spaces
    if (*str == 0) return;  // All spaces

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;  // Trim trailing spaces

    end[1] = '\0';
}

// Function to interpret and generate C code
void interpret_and_generate_code(const char* input_file, const char* output_file) {
    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(output_file, "w");

    if (!in || !out) {
        perror("Failed to open files");
        exit(EXIT_FAILURE);
    }

    // Write the beginning of a standard C program to the output file
    fprintf(out, "#include <stdio.h>\n\nint main() {\n");

    char line[256];
    char var_type[100][10];
    char var_name[100][50];
    int var_count = 0;

    while (fgets(line, sizeof(line), in)) {
        trim(line);
        
        // Handle variable assignments like "x = 5 + 5", "x = 3.14", or "x = \"hello\""
        char name[50];
        char expression[50];


            // Handle list declarations like "list = (1,2,3,4,5)"
        if (sscanf(line, "%49s = (%199[^\n])", name, expression) == 2) {
            trim(expression);
            

            // Remove any trailing parentheses from the extracted expression
            char* end = expression + strlen(expression) - 1;
            if (*end == ')') {
                *end = '\0';  // Remove the trailing parenthesis
            }

            // Generate array declaration in C
            fprintf(out, "    int %s[] = {%s};\n", name, expression);
            strcpy(var_name[var_count], name);
            strcpy(var_type[var_count], "list");
            var_count++;
        }
            // Handle string assignment
        else if (sscanf(line, "%49s = \"%199[^\"]\"", name, expression) == 2) {
            fprintf(out, "    char %s[] = \"%s\";\n", name, expression);
            strcpy(var_name[var_count], name);
            strcpy(var_type[var_count], "string");
            var_count++;
        }
        // Handle numeric assignments
        else if (sscanf(line, "%49s = %199[^\n]", name, expression) == 2) {
            trim(expression);
            if (strchr(expression, '.') != NULL) {
                fprintf(out, "    float %s = %s;\n", name, expression);
                strcpy(var_name[var_count], name);
                strcpy(var_type[var_count], "float");
                var_count++;
            } else if (strspn(expression, "0123456789+-*/ ") == strlen(expression)) {
                fprintf(out, "    int %s = %s;\n", name, expression);
                strcpy(var_name[var_count], name);
                strcpy(var_type[var_count], "int");
                var_count++;
            } else {
                printf("Invalid syntax: %s\n", line);
            }
        }
        // Handle print statements like "print(x)" or "print(list[0])"
        else if (sscanf(line, "print(%49[^)])", name) == 1) {
            trim(name);
            int index = -1;

            // Check for index-based access like "list[0]"
            char list_name[50];
            if (sscanf(name, "%49[^[][%d]", list_name, &index) == 2) {
                // If index-based access is detected, print specific array element
                int found = 0;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(var_name[i], list_name) == 0 && strcmp(var_type[i], "list") == 0) {
                        fprintf(out, "    printf(\"%%d\\n\", %s[%d]);\n", list_name, index);
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    printf("Undefined list: %s\n", list_name);
                }
            } else {
                // Handle normal print statements like "print(x)"
                int found = 0;
                for (int i = 0; i < var_count; i++) {
                    if (strcmp(var_name[i], name) == 0) {
                        if (strcmp(var_type[i], "string") == 0) {
                            fprintf(out, "    printf(\"%%s\\n\", %s);\n", name);
                        } else if (strcmp(var_type[i], "int") == 0) {
                            fprintf(out, "    printf(\"%%d\\n\", %s);\n", name);
                        } else if (strcmp(var_type[i], "float") == 0) {
                            fprintf(out, "    printf(\"%%f\\n\", %s);\n", name);
                        } else if (strcmp(var_type[i], "list") == 0) {
                            // Print the entire list
                            fprintf(out, "    for (int i = 0; i < sizeof(%s) / sizeof(%s[0]); i++) {\n", name, name);
                            fprintf(out, "        printf(\"%%d \", %s[i]);\n", name);
                            fprintf(out, "    }\n    printf(\"\\n\");\n");
                        }
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    printf("Undefined variable: %s\n", name);
                }
            }
        } else {
            printf("Invalid syntax: %s\n", line);
        }
    }

    // End the generated C program
    fprintf(out, "    return 0;\n}\n");

    fclose(in);
    fclose(out);
}

int main() {
    // Input file containing simplified C syntax
    const char* input_file = "simple_program.txt";
    // Output file where the translated C code will be stored
    const char* output_file = "generated_program.c";

    // Interpret and generate the C code
    interpret_and_generate_code(input_file, output_file);

    // Notify user
    printf("Generated C code saved to %s\n", output_file);

    return 0;
}
