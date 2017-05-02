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
  call void @foo()
  ret void
}

define void @foo() {
  call void @bar()
  ret void
}
