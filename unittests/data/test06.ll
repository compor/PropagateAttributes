
define void @qux() {
  call void @bar()
  call void @quz()
  ret void
}

define void @baz() {
  call void @test()
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

define void @test() {
  call void @bar()
  call void @blep()
  ret void
}

define void @blep() {
 ret void
}
