; RUN: opt -load %bindir/${TESTEE_PREFIX}%testee_name${TESTEE_SUFFIX} -propagate-attributes -td-attr=test-attr1 -S < %s | FileCheck %s

; propagate the desired attribute from a function call node in the call graph
; SCC, from a node that contains more than one elements, and nodes with a
; different attribute list which should remain unaffected

define void @qux() {
; CHECK-LABEL: qux
; CHECK: #0
  call void @bar()
  call void @quz()
  ret void
}

define void @baz() {
; CHECK-LABEL: baz
; CHECK-NOT: #0
  ret void
}

define void @fuu() #1 {
; CHECK-LABEL: fuu
; CHECK: #1
; CHECK-NOT: #0
  call void @quz()
  ret void
}

define void @quz() {
; CHECK-LABEL: quz
; CHECK-NOT: #0
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

attributes #1 = { "test-attr2" }
; CHECK: test-attr2


