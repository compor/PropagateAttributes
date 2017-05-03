
define void @qux() {
  call void @bar()
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
  call void @test2()
  ret void
}

define void @test2() {
  call void @baz()
  ret void
}

define void @bar() {
  call void @test1()
  ret void
}

define void @test1() {
  call void @bar()
  call void @blep()
  ret void
}

define void @blep() {
 ret void
}
