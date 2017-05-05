; RUN: opt -load %bindir/${TESTEE_PREFIX}%testee_name${TESTEE_SUFFIX} -propagate-attributes -td-attr=test-attr1 -S < %s | FileCheck %s

; propagate the desired attribute from a function call node in the call graph
; SCC, from a node that contains more than one elements, and a node that gets
; called by a node containing a different kind of attribute. That different
; attribute is also used by nodes which are not in the aforementioned call path,
; thus they should be preserved and a new attribute group should be generated
; for the nodes that require merging of both groups

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

define void @leaf() #1 {
; CHECK-LABEL: leaf
; CHECK: #1
; CHECK-NOT: #0
  ret void
}

define void @fuu() #1 {
; CHECK-LABEL: fuu
; CHECK: #2
; CHECK-NOT: #0
; CHECK-NOT: #1
  call void @quz()
  call void @leaf()
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
; CHECK: test-attr2
; CHECK-NOT: test-attr1

; CHECK-LABEL: attributes #2
; CHECK: test-attr1
; CHECK: test-attr2

