#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Book {
    char title[100];
    char author[100];
    int year;
    int pages;
    float price;
    struct Book* next;
} Book;
static Book* createBook(char* author, char* title, int year, int pages, float price) {
    Book* b = (Book*)malloc(sizeof(Book));
    if (!b) return NULL;
    strcpy(b->author, author);
    strcpy(b->title, title);
    b->year = year;
    b->pages = pages;
    b->price = price;
    b->next = NULL;
    return b;
}
static int readFromFile(const char* filename, Book** pArr, int maxSize) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    char line[256];
    int countBooks = 0;

    while (fgets(line, sizeof(line), f) && countBooks < maxSize) {
        char* a = strtok(line, ",");
        char* t = strtok(NULL, ",");
        char* y = strtok(NULL, ",");
        char* pg = strtok(NULL, ",");
        char* pr = strtok(NULL, ",");

        if (!a || !t || !y || !pg || !pr) continue;

        int year = atoi(y);
        int pages = atoi(pg);
        float price = (float)atof(pr);

        pArr[countBooks] = createBook(a, t, year, pages, price);
        if (pArr[countBooks] != NULL) countBooks++;
    }

    fclose(f);
    return countBooks;
}
static void insertSorted(Book** pHead, Book* pNew) {
    if (*pHead == NULL || strcmp(pNew->title, (*pHead)->title) < 0) {
        pNew->next = *pHead;
        *pHead = pNew;
        return;
    }

    Book* curNode = *pHead;
    while (curNode->next && strcmp(curNode->next->title, pNew->title) < 0) {
        curNode = curNode->next;
    }
    pNew->next = curNode->next;
    curNode->next = pNew;
}

static void printTable(Book* headPtr) {
    printf("------------------------------------------------------------------------------------------------------\n");
    printf("| %-25s | %-25s | %-6s | %-7s | %-8s |\n", "Author", "Title", "Year", "Pages", "Price");
    printf("------------------------------------------------------------------------------------------------------\n");
    while (headPtr) {
        printf("| %-25.25s | %-25.25s | %6d | %7d | %8.2f |\n",
            headPtr->author, headPtr->title, headPtr->year, headPtr->pages, headPtr->price);
        headPtr = headPtr->next;
    }
    printf("------------------------------------------------------------------------------------------------------\n");
}

static float avgPrice(Book* pList) {
    int n = 0;
    float sum = 0.0f;
    while (pList) {
        sum += pList->price;
        n++;
        pList = pList->next;
    }
    if (n == 0) return 0.0f;
    return sum / n;
}

// Ñòâîðþº ÍÎÂÈÉ ñïèñîê êíèã ç price > avg (êîï³¿ âóçë³â)
static Book* filterAboveAvg(Book* src, float avg) {
    Book* resultList = NULL;
    while (src) {
        if (src->price > avg) {
            Book* copyNode = createBook(src->author, src->title, src->year, src->pages, src->price);
            if (copyNode) insertSorted(&resultList, copyNode);
        }
        src = src->next;
    }
    return resultList;
}

static void deletePKL(Book** Head) {
    char bad[] = "PKLpk l";
    while (*Head && strchr(bad, (*Head)->title[0])) {
        Book* tmp = *Head;
        *Head = (*Head)->next;
        free(tmp);
    }
    if (!*Head) return;

    Book* p = *Head;
    while (p->next) {
        if (strchr(bad, p->next->title[0])) {
            Book* tmp = p->next;
            p->next = tmp->next;
            free(tmp);
        }
        else {
            p = p->next;
        }
    }
}

static void freeList(Book** ppHead) {
    while (*ppHead) {
        Book* tmp = *ppHead;
        *ppHead = (*ppHead)->next;
        free(tmp);
    }
}
int main(void) {
    Book* bookArr[200];
    Book* listBooks = NULL;

    int N = readFromFile("books.txt", bookArr, 200);
    if (N == 0) {
        printf("Error reading file.\n");
        return 1;
    }

    printf("================================= Books from file ================================\n");
    for (int iCounter = 0; iCounter < N; iCounter++) {
        bookArr[iCounter]->next = NULL;
        insertSorted(&listBooks, bookArr[iCounter]);
    }
    printTable(listBooks);

    float average_price = avgPrice(listBooks);
    printf("Average price = %.2f\n", average_price);
    Book* sortedAboveAvg = filterAboveAvg(listBooks, average_price);

    printf("================================== Sorted list ==================================\n");
    printTable(sortedAboveAvg);

    printf("================================Inserting new book================================\n");
    Book* pNewBook = createBook("Victor Hugo", "Les Miserables", 1862, 1488, 420.0f);
    insertSorted(&sortedAboveAvg, pNewBook);
    printTable(sortedAboveAvg);

    printf("=======================Deleting books starting with P, K, L=======================\n");
    deletePKL(&sortedAboveAvg);
    printTable(sortedAboveAvg);
    freeList(&sortedAboveAvg);

    return 0;
}