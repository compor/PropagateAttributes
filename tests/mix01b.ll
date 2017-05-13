; RUN: opt -load %bindir/%testeelib -propagate-attributes -ti-attr=noreturn -td-attr=attr1 -S < %s | FileCheck --check-prefix=CHECK-MERGED-ATTR %s

; continuation of previous test, but checking that the target independent
; attribute is also present in the new merged attribute node

; it would have been nice if it was possible to combine the presence of both
; attributes in the same test file and avoid duplication

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

; CHECK-MERGED-ATTR-LABEL: attributes #2
; CHECK-MERGED-ATTR: {{noreturn}}

