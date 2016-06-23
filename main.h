#ifndef MAIN_H
#define MAIN_H

#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <deque>
#include <unordered_map>
#include <cmath>
#include <memory>

using namespace std;

// http://www.stroustrup.com/bs_faq2.html#constraints
template<class T, class B>
struct Derived_from {
  static void constraints(T* p __attribute__((unused))) {
    B* pb __attribute__((unused)) = p;
  }
  Derived_from() { void(*p)(T*) __attribute__((unused)) = constraints; }
};


class ufObject;

typedef vector< shared_ptr< ufObject > > instructions;
typedef deque< shared_ptr< ufObject > > workStack;
typedef unordered_map< string, shared_ptr< ufObject > > dictionary;

template< typename T >
class ufPrimOp {
 public:
  typedef T base_type;
  virtual ~ufPrimOp() {}
  virtual T eval( int left, int right ) = 0;
  virtual string str() = 0;
};

class ufInteger;

class ufAddOp : public ufPrimOp< int > {
 public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left + right;
  }

  string str() {
    return "+";
  }
};


class ufSubOp : public ufPrimOp< int > {
 public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left - right;
  }

  string str() {
    return "-";
  }
};


class ufMulOp : public ufPrimOp< int > {
 public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left * right;
  }

  string str() {
    return "*";
  }
};


class ufDivOp : public ufPrimOp< int > {
 public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return left / right;
  }

  string str() {
    return "/";
  }
};


class ufExpOp : public ufPrimOp< int > {
 public:
  typedef ufInteger value_type;

  int eval( int left, int right ) {
    return static_cast< int >( pow( left, right ) );
  }

  string str() {
    return "^";
  }
};

class ufBoolean;

class ufLtOp : public ufPrimOp< bool > {
 public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left < right;
  }

  string str() {
    return "<";
  }
};

class ufLeOp : public ufPrimOp< bool > {
 public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left <= right;
  }

  string str() {
    return "<=";
  }
};

class ufGtOp : public ufPrimOp< bool > {
 public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left > right;
  }

  string str() {
    return ">";
  }
};

class ufGeOp : public ufPrimOp< bool > {
 public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left >= right;
  }

  string str() {
    return ">=";
  }
};

class ufEqOp : public ufPrimOp< bool > {
 public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left == right;
  }

  string str() {
    return "==";
  }
};

class ufNeOp : public ufPrimOp< bool > {
 public:
  typedef ufBoolean value_type;

  bool eval( int left, int right ) {
    return left != right;
  }

  string str() {
    return "!=";
  }
};

class ufObject {
 public:
  virtual ~ufObject() {}
  virtual void eval( workStack&, dictionary& ) = 0;
  virtual string str() = 0;
};

class ufSymbol : public ufObject {
 public:
 ufSymbol( string s ) : value( s ) {}
  ~ufSymbol() {}

  void eval( workStack& theStack, dictionary& theEnv ) {
    if( theEnv.count( value ) ) {
      auto b = theEnv[ value ];
      b->eval( theStack, theEnv );
    }
    else {
      theStack.push_front( make_shared< ufSymbol >( value ) );
    }
  }

  string str() {
    return value;
  }

 protected:
  string value;
};

class ufInteger : public ufObject {
 public:
 ufInteger( int number ) : value( number ) {}

  void eval( workStack& theStack, dictionary& ) {
    theStack.push_front( make_shared< ufInteger >( value ) );
  }

  string str() {
    return to_string( value );
  }

  int val() {
    return value;
  }

 protected:
  int value;
};

template< typename T >
class ufBinOp : public ufObject, Derived_from< T, ufPrimOp< typename T::base_type > > {
 public:

  void eval( workStack& theStack, dictionary& ) {
    auto right = theStack.front();
    theStack.pop_front();
    auto left = theStack.front();
    theStack.pop_front();

    int rInt = static_cast< ufInteger* >( right.get() )->val();
    int lInt = static_cast< ufInteger* >( left.get() )->val();

    theStack.push_front( make_shared< typename T::value_type >( op.eval( lInt, rInt ) ) );
  }

  string str() {
    return op.str();
  }

 protected:
  T op;
};

const string blockBegin = "{";
const string blockEnd = "}";

class ufBeginBlock : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& ) {
    theStack.push_front( make_shared< ufBeginBlock >() );
  }

  string str() {
    return blockBegin;
  }
};

class ufBlock : public ufObject {
 public:

  void eval( workStack& theStack, dictionary& theEnv ) {
    for( auto i : insns ) {
      i->eval( theStack, theEnv );
    }
  }

  string str() {
    stringstream buffer;

    buffer << blockBegin << " ";

    for( auto i : insns ) {
      buffer << i->str() << " ";
    }

    buffer << blockEnd;
    return buffer.str();
  }

  void addIns( shared_ptr< ufObject >& ins ) {
    insns.insert( insns.begin(), ins );
  }

 protected:
  instructions insns;
};

class ufMkBlock : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& ) {
    auto function = make_shared< ufBlock >();
    while( theStack.front()->str() != blockBegin )  {
      function->addIns( theStack.front() );
      theStack.pop_front();
    }
    theStack.pop_front(); // eat the "("
    theStack.push_front( shared_ptr< ufObject >( function ) );
  }

  string str() {
    return blockEnd;
  }
};

class ufDupOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& ) {
    auto top = static_cast< ufInteger* >( theStack.front().get() )->val();
    auto dup = make_shared< ufInteger >( top );
    theStack.push_front( dup );
  }

  string str() {
    return "dup";
  }
};

class ufSwapOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& ) {
    auto first = theStack.front();
    theStack.pop_front();
    auto second = theStack.front();
    theStack.pop_front();

    theStack.push_front( first );
    theStack.push_front( second );
  }

  string str() {
    return "swap";
  }
};

class ufPopOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& ) {
    theStack.pop_front();
  }

  string str() {
    return "_";
  }
};

class ufAssignOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& theEnv ) {
    auto right =  theStack.front();
    theStack.pop_front();

    auto left = theStack.front();
    theStack.pop_front();

    pair< string, shared_ptr< ufObject > >thing ( left->str(), right );
    theEnv.insert( thing );
  }

  string str() {
    return "=";
  }
};

class ufBoolean : public ufObject {
 public:
  ufBoolean( bool b ) : value( b ) {}

  void eval( workStack& theStack, dictionary& theEnv ) {
    auto thenBlock = theStack.front();
    theStack.pop_front();
    auto elseBlock = theStack.front();
    theStack.pop_front();

    if( value ) {
      thenBlock->eval( theStack, theEnv );
    }
    else {
      elseBlock->eval( theStack, theEnv );
    }
  }

  string str() {
    if( value ) {
      return "true";
    }
    else {
      return "false";
    }
  }

 protected:
  bool value;
};

class ufIfOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& theEnv ) {
    auto cond = theStack.front();
    theStack.pop_front();

    cond->eval( theStack, theEnv );
  }

  string str() {
    return "if";
  }
};

class ufBooleanOp : public ufObject {
 public:
 ufBooleanOp( bool v ) : value( v ) {}

  void eval( workStack& theStack, dictionary& theEnv ) {
    theStack.push_front( make_shared< ufBoolean >( value ) );
  }

  string str() {
    if( value ) {
      return "true";
    }
    else {
      return "false";
    }
  }

 protected:
  bool value;
};

class ufLoopOp : public ufObject {
 public:
  void eval( workStack& theStack, dictionary& theEnv ) {
    auto whileBlock = theStack.front();
    theStack.pop_front();
    auto block = static_cast< ufBlock* >( whileBlock.get() );

    while( theStack.front()->str() == "true" ) {
      theStack.pop_front();
      block->eval( theStack, theEnv );
    }
    theStack.pop_front(); // eat the "false"
  }

  string str() {
    return "loop";
  }
};

#endif
