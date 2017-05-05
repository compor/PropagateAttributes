; RUN: opt -load %bindir/${TESTEE_PREFIX}%testee_name${TESTEE_SUFFIX} -propagate-attributes -ti-attr=noreturn -td-attr=attr1 -S < %s | FileCheck %s

; propagate the desired attribute from a function call node in the call graph
; SCC, from a node that contains more than one elements, up the call chain
; the SCC under test is a terminal node in terms of topological sorting

define void @qux() {
; CHECK-LABEL: qux
; CHECK: #0
; CHECK-NOT: #1
; CHECK-NOT: #2
  call void @bar()
  call void @quz()
  ret void
}

define void @baz() #0 {
; CHECK-LABEL: baz
; CHECK: #0
; CHECK-NOT: #1
; CHECK-NOT: #2
  ret void
}

define void @leaf() #1 {
; CHECK-LABEL: leaf
; CHECK: #1
; CHECK-NOT: #0
; CHECK-NOT: #2
  ret void
}

define void @fuu() {
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
; CHECK-NOT: #1
; CHECK-NOT: #2
  call void @baz()
  ret void
}

define void @bar() {
; CHECK-LABEL: bar
; CHECK: #0
; CHECK-NOT: #1
; CHECK-NOT: #2
  call void @test()
  ret void
}

define void @test() #0 {
; CHECK-LABEL: test
; CHECK: #0
; CHECK-NOT: #1
; CHECK-NOT: #2
  call void @bar()
  ret void
}

attributes #0 = { noreturn }
; CHECK-LABEL: attributes #0
; CHECK: noreturn
; CHECK-NOT: attr1

attributes #1 = { "attr1" }
; CHECK-LABEL: attributes #1
; CHECK: attr1
; CHECK-NOT: noreturn

; CHECK-LABEL: attributes #2
; CHECK: {{"attr1"}}

