#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif

#define CRED_FILE "credentials.txt"
#define STUD_FILE "students.txt"
#define MAXLINE 512
#define MAX_USERS 2000
#define MAX_STUDENTS 5000

typedef struct {
    char username[50];
    char password[50];
    char role[10];
} Credential;

typedef struct {
    int id;                 
    char name[100];
    char gender[10];
    int physics;
    int chemistry;
    int math;
    char grade[4];          
} Student;

void get_password(const char *prompt, char *out, size_t maxlen) {
    printf("%s", prompt);
    fflush(stdout);
    size_t idx = 0;

#ifdef _WIN32
    int ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == 8) { 
            if (idx) { idx--; printf("\b \b"); fflush(stdout); }
        } else if (ch == 3) {
            exit(1);
        } else if (idx + 1 < maxlen) {
            out[idx++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        if (ch == 127 || ch == '\b') {
            if (idx) { idx--; printf("\b \b"); fflush(stdout); }
        } else if (idx + 1 < maxlen) {
            out[idx++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    out[idx] = '\0';
    printf("\n");
}

int load_credentials(Credential creds[], int *count) {
    FILE *f = fopen(CRED_FILE, "r");
    if (!f) return 0;
    char line[MAXLINE];
    *count = 0;
    while (fgets(line, sizeof(line), f)) {
        char *s = strtok(line, "\r\n");
        if (!s) continue;
        char *u = strtok(s, ",");
        char *p = strtok(NULL, ",");
        char *r = strtok(NULL, ",");
        if (u && p && r) {
            strncpy(creds[*count].username, u, sizeof(creds[*count].username)-1);
            strncpy(creds[*count].password, p, sizeof(creds[*count].password)-1);
            strncpy(creds[*count].role, r, sizeof(creds[*count].role)-1);
            (*count)++;
            if (*count >= MAX_USERS) break;
        }
    }
    fclose(f);
    return 1;
}

int load_students(Student arr[], int *n) {
    FILE *f = fopen(STUD_FILE, "r");
    if (!f) { *n = 0; return 0; }
    char line[MAXLINE];
    *n = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) < 3) continue;
        char *tok;
        tok = strtok(line, ",");
        if (!tok) continue;
        arr[*n].id = atoi(tok);

        tok = strtok(NULL, ",");
        if (!tok) continue;
        strncpy(arr[*n].name, tok, sizeof(arr[*n].name)-1);

        tok = strtok(NULL, ",");
        if (tok) strncpy(arr[*n].gender, tok, sizeof(arr[*n].gender)-1);
        else arr[*n].gender[0] = '\0';

        tok = strtok(NULL, ",");
        arr[*n].physics = tok ? atoi(tok) : 0;

        tok = strtok(NULL, ",");
        arr[*n].chemistry = tok ? atoi(tok) : 0;

        tok = strtok(NULL, ",");
        arr[*n].math = tok ? atoi(tok) : 0;

        tok = strtok(NULL, ",\r\n");
        if (tok) strncpy(arr[*n].grade, tok, sizeof(arr[*n].grade)-1);
        else arr[*n].grade[0] = '\0';

        (*n)++;
        if (*n >= MAX_STUDENTS) break;
    }
    fclose(f);
    return 1;
}

int save_students(Student arr[], int n) {
    FILE *f = fopen(STUD_FILE, "w");
    if (!f) { perror("Error opening students file for writing"); return 0; }
    for (int i = 0; i < n; ++i) {
        fprintf(f, "%d,%s,%s,%d,%d,%d,%s\n",
                arr[i].id,
                arr[i].name,
                arr[i].gender,
                arr[i].physics,
                arr[i].chemistry,
                arr[i].math,
                arr[i].grade);
    }
    fclose(f);
    return 1;
}


void flush_stdin(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void print_student(const Student *s) {
    printf("ID: %d\nName: %s\nGender: %s\nPhysics: %d\nChemistry: %d\nMath: %d\nTotal: %d\nAverage: %.2f\nGrade: %s\n",
           s->id, s->name, s->gender, s->physics, s->chemistry, s->math,
           s->physics + s->chemistry + s->math,
           (s->physics + s->chemistry + s->math) / 3.0,
           s->grade);
}


void compute_grade(Student *s) {
    double avg = (s->physics + s->chemistry + s->math) / 3.0;
    if (avg >= 90.0) strncpy(s->grade, "A+", sizeof(s->grade)-1);
    else if (avg >= 80.0) strncpy(s->grade, "A", sizeof(s->grade)-1);
    else if (avg >= 70.0) strncpy(s->grade, "B+", sizeof(s->grade)-1);
    else if (avg >= 60.0) strncpy(s->grade, "B", sizeof(s->grade)-1);
    else if (avg >= 50.0) strncpy(s->grade, "C", sizeof(s->grade)-1);
    else strncpy(s->grade, "F", sizeof(s->grade)-1);
    s->grade[sizeof(s->grade)-1] = '\0';
}


int ci_substr(const char *hay, const char *needle) {
    if (!needle || !*needle) return 1;
    size_t hn = strlen(hay), nn = strlen(needle);
    if (nn > hn) return 0;
    for (size_t i = 0; i + nn <= hn; ++i) {
        size_t j;
        for (j = 0; j < nn; ++j) {
            if (tolower((unsigned char)hay[i+j]) != tolower((unsigned char)needle[j])) break;
        }
        if (j == nn) return 1;
    }
    return 0;
}

int find_student_index_by_id(Student arr[], int n, int id) {
    for (int i = 0; i < n; ++i) if (arr[i].id == id) return i;
    return -1;
}

int find_students_by_name(Student arr[], int n, const char *name_sub, int indices[], int max_indices) {
    int count = 0;
    for (int i = 0; i < n; ++i) {
        if (ci_substr(arr[i].name, name_sub)) {
            if (count < max_indices) indices[count] = i;
            count++;
        }
    }
    return count;
}


int read_mark(const char *prompt, int *out_mark) {
    int m;
    printf("%s", prompt);
    if (scanf("%d", &m) != 1) { flush_stdin(); return 0; }
    if (m < 0 || m > 100) { printf("Marks must be 0-100.\n"); flush_stdin(); return 0; }
    *out_mark = m;
    flush_stdin();
    return 1;
}


void add_student() {
    Student arr[MAX_STUDENTS];
    int n = 0;
    load_students(arr, &n);

    Student s;
    printf("Enter student ID (integer): ");
    if (scanf("%d", &s.id) != 1) { printf("Invalid ID input.\n"); flush_stdin(); return; }
    flush_stdin();
    if (find_student_index_by_id(arr, n, s.id) != -1) {
        printf("A student with ID %d already exists. Add canceled.\n", s.id);
        return;
    }

    printf("Enter name: ");
    fgets(s.name, sizeof(s.name), stdin);
    s.name[strcspn(s.name, "\r\n")] = 0;

    printf("Enter gender: ");
    fgets(s.gender, sizeof(s.gender), stdin);
    s.gender[strcspn(s.gender, "\r\n")] = 0;

    if (!read_mark("Enter marks for Physics (0-100): ", &s.physics)) return;
    if (!read_mark("Enter marks for Chemistry (0-100): ", &s.chemistry)) return;
    if (!read_mark("Enter marks for Math (0-100): ", &s.math)) return;

    compute_grade(&s);

    arr[n++] = s;
    if (save_students(arr, n)) {
        printf("Student added successfully with ID %d. Grade: %s\n", s.id, s.grade);
    } else {
        printf("Failed to save student.\n");
    }
}

void update_student() {
    Student arr[MAX_STUDENTS];
    int n = 0;
    load_students(arr, &n);
    if (n == 0) { printf("No students available.\n"); return; }

    printf("Update by:\n (1) ID \n (2) Name substring ?\n Enter 1 or 2: ");
    int ch; if (scanf("%d", &ch) != 1) { flush_stdin(); return; } flush_stdin();

    int idx = -1;
    if (ch == 1) {
        int id; printf("Enter ID: "); if (scanf("%d", &id) != 1) { printf("Invalid.\n"); flush_stdin(); return; } flush_stdin();
        idx = find_student_index_by_id(arr, n, id);
        if (idx == -1) { printf("Student with ID %d not found.\n", id); return; }
    } else {
        char q[100]; printf("Enter name substring to search: ");
        fgets(q, sizeof(q), stdin); q[strcspn(q, "\r\n")] = 0;
        int indices[50];
        int matches = find_students_by_name(arr, n, q, indices, 50);
        if (matches == 0) { printf("No matches found.\n"); return; }
        printf("Found %d match(es):\n", matches);
        for (int i = 0; i < (matches < 50 ? matches : 50); ++i) {
            int id = arr[indices[i]].id;
            printf("%d) ID: %d, Name: %s\n", i+1, id, arr[indices[i]].name);
        }
        if (matches > 50) printf("...and %d more\n", matches - 50);
        printf("Enter number of the record to edit (1-%d): ", (matches<50?matches:50));
        int choose; if (scanf("%d", &choose) != 1) { printf("Invalid.\n"); flush_stdin(); return; } flush_stdin();
        if (choose < 1 || choose > (matches < 50 ? matches : 50)) { printf("Invalid selection.\n"); return; }
        idx = indices[choose-1];
    }

    printf("Editing record for %s (ID %d)\n", arr[idx].name, arr[idx].id);
    char tmp[200];

    printf("Enter new name (leave blank to keep): ");
    fgets(tmp, sizeof(tmp), stdin);
    if (tmp[0] != '\n' && tmp[0] != '\0') {
        tmp[strcspn(tmp, "\r\n")] = 0;
        strncpy(arr[idx].name, tmp, sizeof(arr[idx].name)-1);
    }

    printf("Enter new gender (leave blank to keep): ");
    fgets(tmp, sizeof(tmp), stdin);
    if (tmp[0] != '\n' && tmp[0] != '\0') {
        tmp[strcspn(tmp, "\r\n")] = 0;
        strncpy(arr[idx].gender, tmp, sizeof(arr[idx].gender)-1);
    }

    
    printf("Do you want to update marks? (y/n): ");
    char chg = getchar(); flush_stdin();
    if (chg == 'y' || chg == 'Y') {
        if (!read_mark("Enter marks for Physics (0-100): ", &arr[idx].physics)) return;
        if (!read_mark("Enter marks for Chemistry (0-100): ", &arr[idx].chemistry)) return;
        if (!read_mark("Enter marks for Math (0-100): ", &arr[idx].math)) return;
        compute_grade(&arr[idx]);
        printf("Updated grade: %s\n", arr[idx].grade);
    }

    if (save_students(arr, n)) printf("Student updated.\n");
    else printf("Failed to save updates.\n");
}

void delete_student() {
    Student arr[MAX_STUDENTS];
    int n = 0;
    load_students(arr, &n);
    if (n == 0) { printf("No students available.\n"); return; }

    printf("Enter ID to delete: ");
    int id; if (scanf("%d", &id) != 1) { printf("Invalid input.\n"); flush_stdin(); return; }
    flush_stdin();
    int idx = find_student_index_by_id(arr, n, id);
    if (idx == -1) { printf("Student not found.\n"); return; }

    printf("Are you sure you want to delete %s (ID %d)? (y/n): ", arr[idx].name, arr[idx].id);
    char confirm = getchar(); flush_stdin();
    if (confirm == 'y' || confirm == 'Y') {
        for (int i = idx; i < n-1; ++i) arr[i] = arr[i+1];
        n--;
        if (save_students(arr, n)) printf("Student deleted.\n");
        else printf("Failed to update storage.\n");
    } else {
        printf("Deletion canceled.\n");
    }
}


void search_student() {
    Student arr[MAX_STUDENTS];
    int n = 0;
    load_students(arr, &n);
    if (n == 0) { printf("No students available.\n"); return; }

    printf("Search by:\n (1) ID \n (2) Name substring ? \n Enter 1 or 2: ");
    int ch; if (scanf("%d", &ch) != 1) { flush_stdin(); return; } flush_stdin();

    if (ch == 1) {
        int id; printf("Enter ID: "); if (scanf("%d", &id) != 1) { printf("Invalid.\n"); flush_stdin(); return; } flush_stdin();
        int idx = find_student_index_by_id(arr, n, id);
        if (idx == -1) printf("Not found.\n");
        else print_student(&arr[idx]);
    } else {
        char q[100];
        printf("Enter name substring to search: ");
        fgets(q, sizeof(q), stdin); q[strcspn(q, "\r\n")] = 0;
        int indices[200];
        int matches = find_students_by_name(arr, n, q, indices, 200);
        if (matches == 0) { printf("No matches found.\n"); return; }

        printf("Found %d match(es):\n", matches);
        for (int i = 0; i < matches; ++i) {
            printf("%d) ID: %d, Name: %s\n", i+1, arr[indices[i]].id, arr[indices[i]].name);
        }
        printf("Enter number of record to view detail (1-%d) or 0 to cancel: ", matches);
        int choose; if (scanf("%d", &choose) != 1) { flush_stdin(); return; } flush_stdin();
        if (choose >= 1 && choose <= matches) {
            print_student(&arr[indices[choose-1]]);
        } else {
            printf("Canceled or invalid selection.\n");
        }
    }
}

void view_all_students() {
    Student arr[MAX_STUDENTS];
    int n = 0;
    load_students(arr, &n);
    if (n == 0) { printf("No students available.\n"); return; }
    for (int i = 0; i < n; ++i) {
        printf("---------\n");
        print_student(&arr[i]);
    }
}

void update_marks() {
    Student arr[MAX_STUDENTS];
    int n = 0;
    load_students(arr, &n);
    if (n == 0) { printf("No students available.\n"); return; }

    printf("Update marks by:\n (1) ID \n (2) Name substring ? \nEnter 1 or 2: ");
    int ch; if (scanf("%d", &ch) != 1) { flush_stdin(); return; } flush_stdin();

    int idx = -1;
    if (ch == 1) {
        int id; printf("Enter ID: "); if (scanf("%d", &id) != 1) { printf("Invalid.\n"); flush_stdin(); return; } flush_stdin();
        idx = find_student_index_by_id(arr, n, id);
    } else {
        char q[100]; printf("Enter name substring to search: ");
        fgets(q, sizeof(q), stdin); q[strcspn(q, "\r\n")] = 0;
        int indices[50];
        int matches = find_students_by_name(arr, n, q, indices, 50);
        if (matches == 0) { printf("No matches found.\n"); return; }
        printf("Found %d match(es):\n", matches);
        for (int i = 0; i < (matches < 50 ? matches : 50); ++i) {
            printf("%d) ID: %d, Name: %s\n", i+1, arr[indices[i]].id, arr[indices[i]].name);
        }
        printf("Select number (1-%d): ", (matches<50?matches:50));
        int choose; if (scanf("%d", &choose) != 1) { printf("Invalid.\n"); flush_stdin(); return; } flush_stdin();
        if (choose < 1 || choose > (matches < 50 ? matches : 50)) { printf("Invalid selection.\n"); return; }
        idx = indices[choose-1];
    }

    if (idx == -1) { printf("Student not found.\n"); return; }

    printf("Current marks for %s: Physics %d, Chemistry %d, Math %d\n",
           arr[idx].name, arr[idx].physics, arr[idx].chemistry, arr[idx].math);

    if (!read_mark("Enter new Physics marks: ", &arr[idx].physics)) return;
    if (!read_mark("Enter new Chemistry marks: ", &arr[idx].chemistry)) return;
    if (!read_mark("Enter new Math marks: ", &arr[idx].math)) return;

    compute_grade(&arr[idx]);
    if (save_students(arr, n)) printf("Marks updated. New grade: %s\n", arr[idx].grade);
    else printf("Failed to save.\n");
}


void add_user() {
    char uname[50], pwd[50], role[10];
    printf("Enter new username: ");
    fgets(uname, sizeof(uname), stdin); uname[strcspn(uname, "\r\n")] = 0;
    get_password("Enter new password: ", pwd, sizeof(pwd));
    printf("Enter role (admin/staff/user): ");
    fgets(role, sizeof(role), stdin); role[strcspn(role, "\r\n")] = 0;
    FILE *f = fopen(CRED_FILE, "a");
    if (!f) { perror("Unable to open credentials file"); return; }
    fprintf(f, "%s,%s,%s\n", uname, pwd, role);
    fclose(f);
    printf("User added.\n");
}


void admin_menu() {
    int opt;
    do {
        printf("=================================================\n");
        printf("                    Admin Menu                   \n");
        printf("=================================================\n");
        printf("1. Add user\n");
        printf("2. Add student\n");
        printf("3. Update student (info/marks)\n");
        printf("4. Delete student\n");
        printf("5. Search student\n");
        printf("6. View all students\n");
        printf("0. Logout\n");
        printf("Choice: "); if (scanf("%d", &opt) != 1) { flush_stdin(); opt = -1; } flush_stdin();
        switch (opt) {
            case 1: add_user(); break;
            case 2: add_student(); break;
            case 3: update_student(); break;
            case 4: delete_student(); break;
            case 5: search_student(); break;
            case 6: view_all_students(); break;
            case 0: printf("Logging out...\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (opt != 0);
}

void staff_menu() {
    int opt;
    do {
        printf("=================================================\n");
        printf("                   Staff Menu                    \n");
        printf("=================================================\n");
        printf("1. Add student\n");
        printf("2. Update student (info/marks)\n");
        printf("3. Update marks\n");
        printf("4. View student record (by name)\n");
        printf("5. Search student\n");
        printf("0. Logout\n");
        printf("Choice: "); if (scanf("%d", &opt) != 1) { flush_stdin(); opt = -1; } flush_stdin();
        switch (opt) {
            case 1: add_student(); break;
            case 2: update_student(); break;
            case 3: update_marks(); break;
            case 4: {
                char name[100];
                printf("Enter name substring to view: ");
                fgets(name, sizeof(name), stdin); name[strcspn(name, "\r\n")] = 0;
                int indices[50];
                Student arr[MAX_STUDENTS]; int n=0;
                load_students(arr, &n);
                int matches = find_students_by_name(arr, n, name, indices, 50);
                if (matches == 0) { printf("Not found.\n"); break; }
                for (int i = 0; i < matches; ++i) {
                    printf("------\n");
                    print_student(&arr[indices[i]]);
                }
                break;
            }
            case 5: search_student(); break;
            case 0: printf("Logging out...\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (opt != 0);
}

void user_menu() {
    int opt;
    do {
        printf("=================================================\n");
        printf("                    User Menu                    \n");
        printf("=================================================\n");
        printf("1. Search student\n");
        printf("2. View marks by student name substring\n");
        printf("3. Show all student records\n");
        printf("0. Logout\n");
        printf("Choice: "); if (scanf("%d", &opt) != 1) { flush_stdin(); opt = -1; } flush_stdin();
        switch (opt) {
            case 1: search_student(); break;
            case 2: {
                char name[100];
                printf("Enter name substring: ");
                fgets(name, sizeof(name), stdin); name[strcspn(name, "\r\n")] = 0;
                Student arr[MAX_STUDENTS]; int n=0;
                load_students(arr, &n);
                int indices[200];
                int matches = find_students_by_name(arr, n, name, indices, 200);
                if (matches == 0) { printf("Not found.\n"); break; }
                for (int i = 0; i < matches; ++i) {
                    Student *s = &arr[indices[i]];
                    printf("ID %d - %s: Physics %d, Chemistry %d, Math %d, Grade %s\n",
                           s->id, s->name, s->physics, s->chemistry, s->math, s->grade);
                }
                break;
            }
            case 3: view_all_students(); break;
            case 0: printf("Logging out...\n"); break;
            default: printf("Invalid choice.\n");
        }
    } while (opt != 0);
}


void login_prompt() {
    Credential creds[MAX_USERS];
    int credCount = 0;
    if (!load_credentials(creds, &credCount)) {
        printf("Warning: credentials file '%s' not found or empty.\n", CRED_FILE);
    }

    char username[50], password[50];
    printf("Username: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\r\n")] = 0;
    get_password("Password: ", password, sizeof(password));

    int found = 0;
    for (int i = 0; i < credCount; ++i) {
        if (strcmp(creds[i].username, username) == 0 && strcmp(creds[i].password, password) == 0) {
            found = 1;
            if (strcasecmp(creds[i].role, "admin") == 0) {
                printf("Welcome Admin %s\n", username);
                admin_menu();
            } else if (strcasecmp(creds[i].role, "staff") == 0) {
                printf("Welcome Staff %s\n", username);
                staff_menu();
            } else {
                printf("Welcome User %s\n", username);
                user_menu();
            }
            break;
        }
    }
    if (!found) {
        printf("Invalid username or password.\n");
    }
}

int main() {
    printf("=================================================\n");
    printf("         Student Record Management System        \n");
    printf("=================================================\n");
    int opt;
    do {
        printf("\nMain Menu:\n");
        printf("1. User Login\n");
        printf("0. Exit\n");
        printf("Choice: ");
        if (scanf("%d", &opt) != 1) { flush_stdin(); opt = -1; }
        flush_stdin();
        switch (opt) {
            case 1:
                login_prompt();
                break;
            case 0:
                printf("Goodbye.\n");
                break;
            default:
                printf("Invalid choice.\n");
        }
    } while (opt != 0);
    return 0;
}

