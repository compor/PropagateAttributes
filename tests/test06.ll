; RUN: opt -load %bindir/${TESTEE_PREFIX}%testee_name${TESTEE_SUFFIX} -propagate-attributes -td-attr=test-attr1 -S < %s | FileCheck %s

; propagate the desired attribute from a function call node in the call graph
; SCC, from a node that contains more than one elements, and a node with a
; different attribute. This different attribute is not used by any other node,
; so it should end up having the merged attributes of both initial groups after
; the transformation

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

define void @fuu() #1 {
; CHECK-LABEL: fuu
; CHECK: #1
; CHECK-NOT: #0
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
; CHECK-LABEL: attributes #0
; CHECK: test-attr1
; CHECK-NOT: test-attr2

attributes #1 = { "test-attr2" }
; CHECK-LABEL: attributes #1
; CHECK: test-attr1
; CHECK: test-attr2


