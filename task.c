#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MIN_YEAR   1000
#define MAX_YEAR   2025
#define OUTPUT_FILE "books_result.txt"
typedef struct Book {
    char  title[100];
    char  author[100];
    int   year;
    int   pages;
    float price;
    struct Book* next;
} Book;
static void TrimSpaces(char* s) {
    if (!s) return;
    int len = (int)strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\n' ||
        s[len - 1] == '\r' || s[len - 1] == '\t'))
        s[--len] = '\0';
    int start = 0;
    while (s[start] == ' ' || s[start] == '\t') start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
}
static int ValidateBook(const char* author, const char* title,
    int year, int pages, float price) {
    if (!author || strlen(author) == 0) {
        fprintf(stderr, "  [!] Помилка: порожнє поле 'author'.\n");
        return 0;
    }
    if (!title || strlen(title) == 0) {
        fprintf(stderr, "  [!] Помилка: порожнє поле 'title'.\n");
        return 0;
    }
    if (year < MIN_YEAR || year > MAX_YEAR) {
        fprintf(stderr, "  [!] Помилка: некоректний рік '%d' (очікується %d–%d).\n",
            year, MIN_YEAR, MAX_YEAR);
        return 0;
    }
    if (pages <= 0) {
        fprintf(stderr, "  [!] Помилка: некоректна кількість сторінок '%d'.\n", pages);
        return 0;
    }
    if (price <= 0.0f) {
        fprintf(stderr, "  [!] Помилка: некоректна ціна '%.2f'.\n", price);
        return 0;
    }
    return 1;
}
static Book* CreateBook(char* author, char* title,
    int year, int pages, float price) {
    Book* b = (Book*)malloc(sizeof(Book));
    if (!b) {
        fprintf(stderr, "  [!] Помилка: не вдалося виділити пам'ять.\n");
        return NULL;
    }
    strcpy(b->author, author);
    strcpy(b->title, title);
    b->year = year;
    b->pages = pages;
    b->price = price;
    b->next = NULL;
    return b;
}
static void InsertSorted(Book** pHead, Book* pNew) {
    if (*pHead == NULL || strcmp(pNew->title, (*pHead)->title) < 0) {
        pNew->next = *pHead;
        *pHead = pNew;
        return;
    }
    Book* cur = *pHead;
    while (cur->next && strcmp(cur->next->title, pNew->title) < 0)
        cur = cur->next;
    pNew->next = cur->next;
    cur->next = pNew;
}
static void FreeList(Book** ppHead) {
    while (*ppHead) {
        Book* tmp = *ppHead;
        *ppHead = (*ppHead)->next;
        free(tmp);
    }
}
static int ReadFromFile(const char* filename, Book** pHead) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "[!] Помилка: файл '%s' не знайдено.\n", filename);
        return 0;
    }
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) {
        fprintf(stderr, "[!] Помилка: файл '%s' порожній.\n", filename);
        fclose(f);
        return 0;
    }
    rewind(f);
    char line[256];
    int  bookCount = 0;
    int  lineNumber = 0;
    while (fgets(line, sizeof(line), f)) {
        lineNumber++;
        TrimSpaces(line);
        if (strlen(line) == 0) continue;
        char* a = strtok(line, ",");
        char* t = strtok(NULL, ",");
        char* y = strtok(NULL, ",");
        char* pg = strtok(NULL, ",");
        char* pr = strtok(NULL, ",");

        if (!a || !t || !y || !pg || !pr) {
            fprintf(stderr, "  [!] Рядок %d: недостатньо полів, пропускаємо.\n",
                lineNumber);
            continue;
        }
        TrimSpaces(a);  TrimSpaces(t);
        TrimSpaces(y);  TrimSpaces(pg);  TrimSpaces(pr);
        int   year = atoi(y);
        int   pages = atoi(pg);
        float price = (float)atof(pr);
        if (!ValidateBook(a, t, year, pages, price)) {
            fprintf(stderr, "  [!] Рядок %d: запис пропущено через помилку.\n",
                lineNumber);
            continue;
        }
        Book* b = CreateBook(a, t, year, pages, price);
        if (b) {
            InsertSorted(pHead, b);
            bookCount++;
        }
    }

    fclose(f);
    return bookCount;
}
static int WriteToFile(const char* filename, Book* pHead) {
    FILE* f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "[!] Помилка: не вдалося відкрити '%s' для запису.\n",
            filename);
        return 0;
    }
    int written = 0;
    while (pHead) {
        fprintf(f, "%s,%s,%d,%d,%.2f\n",
            pHead->author, pHead->title,
            pHead->year, pHead->pages, pHead->price);
        written++;
        pHead = pHead->next;
    }
    fclose(f);
    printf("[i] Збережено %d записів у файл '%s'.\n", written, filename);
    return 1;
}
static void PrintTable(Book* head) {
    printf("------------------------------------------------------------------------------------------------------\n");
    printf("| %-25s | %-25s | %-6s | %-7s | %-8s |\n",
        "Author", "Title", "Year", "Pages", "Price");
    printf("------------------------------------------------------------------------------------------------------\n");
    if (!head) {
        printf("| %-96s |\n", "  (список порожній)");
    }
    while (head) {
        printf("| %-25.25s | %-25.25s | %6d | %7d | %8.2f |\n",
            head->author, head->title,
            head->year, head->pages, head->price);
        head = head->next;
    }
    printf("------------------------------------------------------------------------------------------------------\n");
}
static float AvgPrice(Book* pList) {
    int   n = 0;
    float sum = 0.0f;
    while (pList) { sum += pList->price; n++; pList = pList->next; }
    return (n == 0) ? 0.0f : sum / n;
}
static Book* FilterAboveAvg(Book* src, float avg) {
    Book* filteredList = NULL;
    while (src) {
        if (src->price > avg) {
            Book* copy = CreateBook(src->author, src->title,
                src->year, src->pages, src->price);
            if (copy) InsertSorted(&filteredList, copy);
        }
        src = src->next;
    }
    return filteredList;
}
static int StartsWithPKL(const char* title) {
    char c = title[0];
    return (c == 'П' || c == 'п' || c == 'К' || c == 'к' || c == 'Л' || c == 'л' ||
        c == 'P' || c == 'p' || c == 'K' || c == 'k' || c == 'L' || c == 'l');
}
static void DeletePKL(Book** pHead) {
    while (*pHead && StartsWithPKL((*pHead)->title)) {
        Book* tmp = *pHead;
        *pHead = (*pHead)->next;
        free(tmp);
    }
    if (!*pHead) return;
    Book* p = *pHead;
    while (p->next) {
        if (StartsWithPKL(p->next->title)) {
            Book* tmp = p->next;
            p->next = tmp->next;
            free(tmp);
        }
        else {
            p = p->next;
        }
    }
}
int main(void) {
    Book* listBooks = NULL;
    Book* filteredList = NULL;
    int N = ReadFromFile("books.txt", &listBooks);
    if (N == 0) {
        printf("Програма завершена: немає даних для обробки.\n");
        return 1;
    }
    printf("[i] Прочитано %d книг з файлу.\n\n", N);
    printf("================================= Books from file =================================\n");
    PrintTable(listBooks);
    float avgPriceVal = AvgPrice(listBooks);
    printf("Average price = %.2f\n\n", avgPriceVal);
    filteredList = FilterAboveAvg(listBooks, avgPriceVal);
    printf("================================== Sorted list ====================================\n");
    PrintTable(filteredList);
    printf("================================ Inserting new book ===============================\n");
    Book* pNewBook = CreateBook("Victor Hugo", "Les Miserables", 1862, 1488, 420.0f);
    if (pNewBook) InsertSorted(&filteredList, pNewBook);
    PrintTable(filteredList);
    printf("==================== Deleting books starting with П, К, Л ========================\n");
    DeletePKL(&filteredList);
    PrintTable(filteredList);
    WriteToFile(OUTPUT_FILE, filteredList);
    FreeList(&filteredList);
    FreeList(&listBooks);
    return 0;
}