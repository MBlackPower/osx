// Build don't link:

struct A 
{
  void f() {}

  template <class U>
  void f() {}
};


template <class T>
struct B
{
  void f() {}

  template <class U>
  void f() {}
};

template struct B<int>;

struct C 
{
  template <class U>
  void f() {}

  template <class U>
  void f() {}  // ERROR - redeclaration
};


template <class T, class U>
struct D
{
  void f(T);
  void f(U);
};

template struct D<int, double>;

template <class T, class U>
struct D2
{
  void f(T);
  void f(U); // ERROR - redeclaration 
};

template struct D2<int, int>; 

struct E
{
  void f(); 
  void f(); // ERROR - redeclaration
};

