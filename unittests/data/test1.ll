define void @test() {
  %i = alloca i32, align 4
  %a = alloca i32, align 4
  store i32 100, i32* %i, align 4
  store i32 0, i32* %a, align 4
  ret void
}
