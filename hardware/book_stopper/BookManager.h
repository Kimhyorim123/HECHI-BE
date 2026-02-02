#ifndef BOOK_MANAGER_H
#define BOOK_MANAGER_H

#include "Book.h"


class BookManager {
private:
  static const int MAX_BOOKS = 100;

  Book books[MAX_BOOKS];
  int pointer;
  int count;

  BookManager(): pointer(0), count(0) {}

public:
  static BookManager& getInstance() {
    static BookManager instance;
    return instance;
  }

  BookManager(const BookManager&) = delete;
  BookManager& operator=(const BookManager&) = delete;

  void init() {
    pointer = 0;
    count = 0;
    for (int i = 0; i < MAX_BOOKS; i++) books[i] = Book();

    registerBook(0, "select book", 0);
  }

  bool registerBook(int id, const String& title, int page) {
    if (count >= MAX_BOOKS) return false;
    books[count++] = Book(id, title, page);
    return true;
  }

  void setPointer(int pos) { pointer = pos % count; }

  void rotateLeft()  { if (count > 0) pointer = (pointer + 1) % count; }
  void rotateRight() { if (count > 0) pointer = (pointer - 1 + count) % count; }

  Book* getCurrentBook()             { return (count == 0) ? nullptr : &books[pointer]; }
  const Book* getCurrentBook() const { return (count == 0) ? nullptr : &books[pointer]; }

  void setPage(int page) { books[pointer].page = page; }
  void turnPageForward()  { books[pointer].page += 1; }
  void turnPageBackward() { books[pointer].page -= (books[pointer].page > 0); }
};

#endif