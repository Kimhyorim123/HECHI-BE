#ifndef BOOK_H
#define BOOK_H

class Book {
public:
  int id;
  String title;
  int page;

  Book(): id(-1), title(""), page(0) {}
  Book(int id, const String& title, int page): id(id), title(title), page(page) {}
};

#endif