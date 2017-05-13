; RUN: opt -load %bindir/%testeelib -propagate-attributes -td-attr=test-attr1 -S < %s | FileCheck %s

; propagate the desired attribute from a function call node in the call graph
; SCC, from a node that contains more than one elements, and an another SCC
; which contains the same attribute

define void @qux() {
; CHECK-LABEL: qux
; CHECK: #0
  call void @bar()
  call void @quz()
  ret void
}

define void @baz() #0 {
; CHECK-LABEL: baz
; CHECK: #0
  ret void
}

define void @fuu() {
; CHECK-LABEL: fuu
; CHECK: #0
  call void @quz()
  ret void
}

define void @quz() {
; CHECK-LABEL: quz
; CHECK: #0
  call void @baz()
  ret void
}

define void @bar() {
; CHECK-LABEL: bar
; CHECK: #0
  call void @test()
  ret void
}

define void @test() #0 {
; CHECK-LABEL: test
; CHECK: #0
  call void @bar()
  ret void
}

attributes #0 = { "test-attr1" }
; CHECK: test-attr1

