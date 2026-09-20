"""Real Vector value/lifetime controls using the actual header and TU bodies.

Only the platform precompiled-header include is omitted for the host build.
No Vector or arithmetic doubles are used. Separate translation units preserve
the real opaque close call; disabled copy elision exercises returned-value
independence. Host floating-point tests cover representative values, not the
original x87 precision, NaN payloads, exception flags or native calling ABI.
"""

from pathlib import Path
import shutil
import subprocess

import pytest


ROOT = Path(__file__).resolve().parents[2]


def product():
    header = (ROOT / "src/vector.h").read_text()
    source = (ROOT / "src/vector.cpp").read_text()
    assert source.count('#include "stdafx.h"') == 1
    return header, source.replace('#include "stdafx.h"', '')


HARNESS = r"""
#include "vector.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <type_traits>
#include <utility>

void check_at(bool condition, int line) {
    if (!condition) { std::fprintf(stderr,"behavior assertion line %d\n",line); std::exit(91); }
}
#define check(x) check_at((x),__LINE__)
void set(Vector &v,float x,float y,float z) { v.x_=x; v.y_=y; v.z_=z; }
void values(const Vector &v,float x,float y,float z) {
    check(v.x_==x && v.y_==y && v.z_==z);
}
void approximately(const Vector &v,float x,float y,float z) {
    check(std::fabs(v.x_-x)<0.000001f && std::fabs(v.y_-y)<0.000001f && std::fabs(v.z_-z)<0.000001f);
}
static_assert(std::is_same<decltype(std::declval<Vector&>() + std::declval<Vector&>()),Vector>::value,
              "addition returns an independent class value");
static_assert(std::is_same<decltype(std::declval<Vector&>() - std::declval<Vector&>()),Vector>::value,
              "subtraction returns an independent class value");
static_assert(std::is_same<decltype(std::declval<Vector&>().cross(std::declval<Vector&>())),Vector>::value,
              "cross returns a class value");
static_assert(std::is_same<decltype(std::declval<Vector&>().scale(1.0f)),Vector>::value,
              "mutating scale also returns a class value");
static_assert(std::is_same<decltype(std::declval<Vector&>() += std::declval<Vector&>()),Vector&>::value,
              "compound addition returns a reference");
static_assert(std::is_same<decltype(std::declval<Vector&>() -= std::declval<Vector&>()),Vector&>::value,
              "compound subtraction returns a reference");

void construction_and_cleanup() {
    Vector array[8];
    for(int i=0;i<8;++i) {
        values(array[i],0,0,0);
        check(!std::signbit(array[i].x_) && !std::signbit(array[i].y_) && !std::signbit(array[i].z_));
        set(array[i],float(i+1),float(i+2),float(i+3));
    }
    array[3].close(); values(array[3],0,0,0);
    values(array[2],3,4,5); values(array[4],5,6,7);
    array[3].close(); values(array[3],0,0,0);

    // Observe only bytes of the still-live backing character array after
    // explicit destruction, never members of the ended Vector lifetime.
    alignas(Vector) unsigned char storage[sizeof(Vector)];
    std::memset(storage,0xA7,sizeof(storage));
    Vector *object=new(storage) Vector;
    values(*object,0,0,0); set(*object,17,-23,41);
    object->~Vector();
    const float zero[3]={0.0f,0.0f,0.0f};
    check(std::memcmp(storage,zero,sizeof(storage))==0);
    object=new(storage) Vector;
    values(*object,0,0,0); object->~Vector();
}

void independent_values() {
    Vector a,b; set(a,2,-3,5); set(b,-7,11,13);
    {
        Vector sum=a+b; values(sum,-5,8,18);
        Vector difference=a-b; values(difference,9,-14,-8);
        Vector cross=a.cross(b); values(cross,-94,-61,1);
        values(a,2,-3,5); values(b,-7,11,13);
        sum.x_=91; difference.y_=92; cross.z_=93;
        values(a,2,-3,5); values(b,-7,11,13);
        sum.close(); difference.close(); cross.close();
        values(a,2,-3,5); values(b,-7,11,13);
    }
    values(a,2,-3,5); values(b,-7,11,13);
    Vector twice=a+a; values(twice,4,-6,10);
    Vector zero=a-a; values(zero,0,0,0);
    Vector parallel=a.cross(a); values(parallel,0,0,0);
    values(a,2,-3,5);
    a=a+b; values(a,-5,8,18); values(b,-7,11,13);
    a=a-b; values(a,2,-3,5);
    a=a.cross(b); values(a,-94,-61,1); values(b,-7,11,13);
    const Vector &left=a, &right=b;
    check(left.dot(right)==0); // 658 - 671 + 13
}

void compounds_and_scale() {
    Vector a,b; set(a,2,-3,5); set(b,-7,11,13);
    Vector &added=(a+=b); check(&added==&a); values(a,-5,8,18); values(b,-7,11,13);
    added.x_=3; values(a,3,8,18);
    Vector &subtracted=(a-=b); check(&subtracted==&a); values(a,10,-3,5);
    subtracted.z_=7; values(a,10,-3,7);
    check(&(a+=a)==&a); values(a,20,-6,14);
    check(&(a-=a)==&a); values(a,0,0,0);
    set(a,2,-3,5);
    {
        Vector result=a.scale(-0.5f);
        values(a,-1,1.5f,-2.5f); values(result,-1,1.5f,-2.5f);
        result.y_=77; values(a,-1,1.5f,-2.5f);
        result.close(); values(a,-1,1.5f,-2.5f);
    }
    a.scale(2.0f); values(a,-2,3,-5); // discarded return must not clear the receiver
    a=a.scale(-1.0f); values(a,2,-3,5);
    a.scale(0.0f); values(a,0,0,0);
    set(a,2,-3,5); check(a.dot(b)==18); check(a.dot(a)==38);
}

void normalization_and_equality() {
    Vector a,b; set(a,3,4,0);
    Vector normalized=a.normalize(); approximately(normalized,0.6f,0.8f,0);
    values(a,3,4,0); normalized.x_=77; values(a,3,4,0);
    set(a,-2,0,0); Vector negative=a.normalize(); values(negative,-1,0,0); values(a,-2,0,0);
    set(a,-0.0f,1,0); Vector unit=a.normalize(); values(unit,0,1,0);
    check(std::signbit(unit.x_)); unit.close(); values(a,0,1,0);
    set(a,0,0,0); Vector zero=a.normalize();
    check(std::isnan(zero.x_) && std::isnan(zero.y_) && std::isnan(zero.z_));
    values(a,0,0,0); // no invented zero-length normalization guard
    const float nan=std::numeric_limits<float>::quiet_NaN();
    set(a,nan,-7,9); Vector unordered=a.normalize();
    check(std::isnan(unordered.x_) && unordered.y_==-7 && unordered.z_==9);
    unordered.y_=81; check(std::isnan(a.x_) && a.y_==-7 && a.z_==9);
    set(a,2,-3,5); b=a; check(a==b);
    b.x_=3; check(!(a==b)); b=a; b.y_=4; check(!(a==b)); b=a; b.z_=6; check(!(a==b));
    set(a,-0.0f,0,0); set(b,0,-0.0f,0); check(a==b);
    // Original equality accepts unordered individual components, but an
    // ordered mismatch elsewhere still rejects the pair.
    set(a,nan,-3,5); set(b,2,-3,5); check(a==b && b==a);
    b.y_=4; check(!(a==b) && !(b==a));
    set(a,2,nan,5); set(b,2,9,5); check(a==b);
    b.z_=6; check(!(a==b));
    set(a,2,-3,nan); set(b,2,-3,99); check(a==b);
    b.x_=4; check(!(a==b));
}
int main() {
    construction_and_cleanup(); independent_values();
    compounds_and_scale(); normalization_and_equality();
}
"""


def run(tmp_path, header, source):
    compiler=shutil.which("g++")
    if not compiler:
        pytest.skip("host g++ required")
    (tmp_path / "vector.h").write_text(header)
    (tmp_path / "vector_product.cpp").write_text(source)
    (tmp_path / "values.cpp").write_text(HARNESS)
    executable=tmp_path / "values"
    result=subprocess.run([compiler,"-std=c++11","-O1","-fno-elide-constructors",
        "-fsanitize=undefined","-fno-sanitize-recover=undefined","-D__cdecl=",
        "-I",str(tmp_path),str(tmp_path / "vector_product.cpp"),str(tmp_path / "values.cpp"),
        "-o",str(executable)],capture_output=True,text=True,timeout=30)
    assert result.returncode==0,result.stdout+result.stderr
    return subprocess.run([str(executable)],capture_output=True,text=True,timeout=10)


def test_actual_vector_values(tmp_path):
    result=run(tmp_path,*product())
    assert result.returncode==0,result.stdout+result.stderr


@pytest.mark.parametrize("damage",["wrong_value_return","mutated_input","cross_math",
    "wrong_compound_reference","nonmutating_scale","close_component","missing_destructor_close",
    "zero_normalize_guard"])
def test_vector_value_damage(tmp_path,damage):
    header,source=product()
    changes={
        "wrong_value_return":("    result.z_ = z_ - right.z_;\n    return result;",
                              "    result.z_ = z_ - right.z_;\n    return *this;"),
        "mutated_input":("    result.x_ = right.x_ + x_;","    result.x_ = x_ = right.x_ + x_;"),
        "cross_math":("    result.y_ = right.x_ * z_ - x_ * right.z_;",
                      "    result.y_ = x_ * right.z_ - right.x_ * z_;"),
        "wrong_compound_reference":("    z_ += right.z_;\n    return *this;",
                                    "    z_ += right.z_;\n    return right;"),
        "nonmutating_scale":("    x_ *= scalar;\n    y_ *= scalar;\n    z_ *= scalar;\n    return *this;",
                             "    Vector result = *this;\n    result.x_ *= scalar;\n    result.y_ *= scalar;\n    result.z_ *= scalar;\n    return result;"),
        "close_component":("void Vector::close() {\n    x_ = 0.0f;\n    y_ = 0.0f;\n    z_ = 0.0f;",
                           "void Vector::close() {\n    x_ = 0.0f;\n    y_ = 0.0f;\n    z_ = 1.0f;"),
        "zero_normalize_guard":("    float reciprocal = 1.0f / length;",
                                "    if (length == 0.0f) return *this;\n    float reciprocal = 1.0f / length;"),
    }
    if damage=="missing_destructor_close":
        old="inline Vector::~Vector() {\n  close();\n}"
        assert header.count(old)==1
        header=header.replace(old,"inline Vector::~Vector() {}")
    else:
        old,new=changes[damage]; assert source.count(old)==1,damage
        source=source.replace(old,new)
    result=run(tmp_path,header,source)
    assert result.returncode==91,result.stdout+result.stderr
