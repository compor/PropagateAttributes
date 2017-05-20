; RUN: opt -load %bindir/%testeelib -propagate-attributes -pattr-ti-attr=noreturn -pattr-td-attr=attr1 -pattr-stats=pattr -S < %s
; RUN: diff <(sort pattr-filtered-attr1.txt) <(sort %outputdatadir/mix01c-filtered-attr1.txt)
; RUN: diff <(sort pattr-propagated-attr1.txt) <(sort %outputdatadir/mix01c-propagated-attr1.txt)
; RUN: diff <(sort pattr-filtered-noreturn.txt) <(sort %outputdatadir/mix01c-filtered-noreturn.txt)
; RUN: diff <(sort pattr-propagated-noreturn.txt) <(sort %outputdatadir/mix01c-propagated-noreturn.txt)

; propagate the desired attribute from a function call node in the call graph
; SCC, from a node that contains more than one elements, up the call chain
; the SCC under test is a terminal node in terms of topological sorting

define void @qux() {
  call void @bar()
  call void @quz()
  ret void
}

define void @baz() #0 {
  ret void
}

define void @leaf() #1 {
  ret void
}

define void @fuu() {
  call void @quz()
  call void @leaf()
  ret void
}

define void @quz() {
  call void @baz()
  ret void
}

define void @bar() {
  call void @test()
  ret void
}

define void @test() #0 {
  call void @bar()
  ret void
}

attributes #0 = { noreturn }

attributes #1 = { "attr1" }

