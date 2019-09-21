#ifndef VIS_EQUALITY_H
#define VIS_EQUALITY_H
#include "parser_nodes.h"
#include "vis_walker.h"
#include "tokens.h"
#include <algorithm>

struct vis_expr_type : public vis_walker {
   tok get_type_of(expr* e)
   {
      e->enter(this);
      return type;
   }

   void enter(var       *p) override { type = tok::id;        }
   void enter(num       *p) override { type = tok::num_lit;   }
   void enter(str       *p) override { type = tok::str_lit;   }
   void enter(dif       *p) override { type = tok::minus;     }
   void enter(_or       *p) override { type = tok::_or;       }
   void enter(therefore *p) override { type = tok::therefore; }
   void enter(_and      *p) override { type = tok::_and;      }
   void enter(more      *p) override { type = tok::more;      }
   void enter(_less     *p) override { type = tok::less;      }
   void enter(more_eq   *p) override { type = tok::more_eq;   }
   void enter(less_eq   *p) override { type = tok::less_eq;   }
   void enter(_equal    *p) override { type = tok::equal;     }
   void enter(_not      *p) override { type = tok::_not;      }
   void enter(un_dif    *p) override { type = tok::un_minus;  }
   void enter(un_add    *p) override { type = tok::un_pluss;  }
   void enter(call      *p) override { type = tok::_func;     }
   void enter(ifel      *p) override { type = tok::_if;       }
   void enter(for_each_ *p) override { type = tok::_for;      }
   void enter(ret       *p) override { type = tok::_return;   }
   void enter(assert    *p) override { type = tok::_assert;   }
   void enter(lam       *p) override { type = tok::lam;       }

private:
   tok type;
};


struct vis_equality : public vis_walker {
   
   bool is_in(expr* A, expr* B)
   {
      equal = true;
      _A_start = _A = A;
      _B = B;
      
      if (equal_types(_A, _B)) {
         _A->enter(this);
      } else {
         equal = false;
      }

      return equal;
   }


   bool equal_types(expr* A, expr* B)
   {
      return expert.get_type_of(A) == expert.get_type_of(B);
   }


   template<typename T> void check_equality_of_not_commut_bin_op(T* p)
   {
      T* q = (T*)_B;

      if (p->l == nullptr && p->r == nullptr) {
         equal = true;
         return;
      }

      equal = (p->l == nullptr || equal_types(p->l, q->l)) && (p->r == nullptr || equal_types(p->r, q->r));

      if (!equal) { return; }

      if (p->l != nullptr) {
         _B = q->l;
         p->l->enter(this);
      }

      if (!equal) return;

      if (p->r != nullptr) {
         _B = q->r;
         p->r->enter(this);
      }

      if (!equal) return;
   }


   template<typename T> void check_equality_of_commut_bin_op(T* p)
   {
      T* q = (T*)_B;

      if (p->l == nullptr && p->r == nullptr) {
         equal = true;
         return;
      }

      bool first  = (p->l == nullptr || equal_types(p->l, q->l)) && (p->r == nullptr || equal_types(p->r, q->r));
      bool second = (p->l == nullptr || equal_types(p->l, q->r)) && (p->r == nullptr || equal_types(p->r, q->l));

      equal = first || second;

      if (!equal) { return; }

      if (p->l != nullptr) {
         _B = first ? q->l : q->r;
         p->l->enter(this);
      }

      if (!equal) return;

      if (p->r != nullptr) {
         _B = first ? q->r : q->l;
         p->r->enter(this);
      }

      if (!equal) return;
   }


   template<typename T> void check_un_op_equality(T* p)
   {
      T* q = (T*)_B;

      if (p->r == nullptr) {
         equal = true;
         return;
      }

      equal = equal_types(p->r, q->r);

      if (!equal) return;

      if (p->r != nullptr) {
         _B = q->r;
         p->r->enter(this);
      }

      if (!equal) return;
   }


   void enter(dif    *p) override { check_equality_of_not_commut_bin_op(p); }
   void enter(_and   *p) override { check_equality_of_commut_bin_op(p); }
   void enter(_or    *p) override { check_equality_of_commut_bin_op(p); }
   void enter(_not   *p) override { check_un_op_equality(p); }
   void enter(un_dif *p) override { check_un_op_equality(p); }
   void enter(un_add *p) override { check_un_op_equality(p); }
private:
   bool equal = false;

   expr* _A_start = nullptr;
   expr* _A       = nullptr;
   expr* _B       = nullptr;

   vis_expr_type expert;
};

#endif // ifndef VIS_EQUALITY_H