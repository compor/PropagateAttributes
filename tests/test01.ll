; RUN: false
; XFAIL: *

define void @qux() {
  call void @bar()
  call void @quz()
  ret void
}

define void @baz() {
  ret void
}

define void @fuu() {
  call void @quz()
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

attributes #0 = { "test-attr1" }

