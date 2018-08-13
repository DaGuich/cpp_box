#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"

#include <arm_thing/arm.hpp>

template<bool B> bool static_test()
{
  static_assert(B);
  return B;
}

template<typename... T> constexpr auto run_instruction(T... instruction)
{
  arm_thing::System system{};
  system.PC() = 4;
  (system.process(instruction), ...);
  return system;
}

template<std::size_t N> constexpr auto run_code(std::uint32_t start, std::array<std::uint8_t, N> memory)
{
  arm_thing::System system{ memory };
  system.run(start);
  return system;
}

template<typename... T> constexpr auto run(T... bytes)
{
  std::array<uint8_t, sizeof...(T)> data{ static_cast<std::uint8_t>(bytes)... };
  arm_thing::System system{ data };
  system.run(0);
  return system;
}


TEST_CASE("test always executing jump")
{
  constexpr auto systest2 = run_instruction(arm_thing::Instruction{ 0b1110'1010'0000'0000'0000'0000'0000'1111 });
  REQUIRE(static_test<systest2.PC() == 72>());
  REQUIRE(static_test<systest2.registers[14] == 0>());
}

TEST_CASE("test always executing jump with saved return")
{
  constexpr auto systest3 = run_instruction(arm_thing::Instruction{ 0b1110'1011'0000'0000'0000'0000'0000'1111 });
  REQUIRE(static_test<systest3.PC() == 72>());
  REQUIRE(static_test<systest3.registers[14] == 4>());
}

TEST_CASE("test carry flag")
{
  //   4:	e3e01000 	mvn	r1, #0
  //  10:	e2911001 	add	r1, r1, #1 ; and set flags
  constexpr auto systest = run_instruction(arm_thing::Instruction{ 0xe3e01000 }, arm_thing::Instruction{ 0xe2911001 });
  REQUIRE(static_test<systest.registers[1] == 0x0>());
  REQUIRE(static_test<systest.c_flag()>());
  REQUIRE(static_test<systest.z_flag()>());
}


TEST_CASE("register setups and moves")
{
  //  0:	e3a02d71 	mov	r2, #7232	; 0x1c40
  //   4:	e3a00000 	mov	r0, #0
  //   8:	e3a01901 	mov	r1, #16384	; 0x4000
  //   c:	e3822903 	orr	r2, r2, #49152	; 0xc000
  //  10:	e4c10003 	strb	r0, [r1], #3
  //  14:	e2800001 	add	r0, r0, #1
  //  18:	e1510002 	cmp	r1, r2

  constexpr auto systest = run_instruction(arm_thing::Instruction{ 0xe3a02d71 },
                                           arm_thing::Instruction{ 0xe3a00000 },
                                           arm_thing::Instruction{ 0xe3a01901 },
                                           arm_thing::Instruction{ 0xe3822903 },
                                           arm_thing::Instruction{ 0xe4c10003 },
                                           arm_thing::Instruction{ 0xe2800001 },
                                           arm_thing::Instruction{ 0xe1510002 });

  REQUIRE(static_test<systest.registers[0] == 1>());
  REQUIRE(static_test<systest.registers[1] == 0x4003>());
  REQUIRE(static_test<systest.registers[2] == 0x4000 + 100 * 100 * 4>());
  REQUIRE(static_test<systest.c_flag() == false>());
}


TEST_CASE("CMP with carry")
{
  constexpr auto systest = run_instruction(arm_thing::Instruction{ 0xe3a01001 },   // mov r1, #1
                                           arm_thing::Instruction{ 0xe3a02001 },   // mov r2, #1
                                           arm_thing::Instruction{ 0xe1510002 });  // cmp r1, r2
  // Should be true if no borrow occurred
  REQUIRE(static_test<systest.c_flag() == true>());
}

TEST_CASE("CMP with carry 2")
{
  constexpr auto systest = run_instruction(arm_thing::Instruction{ 0xe3a01001 },   // mov r1, #1
                                           arm_thing::Instruction{ 0xe3a02000 },   // mov r2, #0
                                           arm_thing::Instruction{ 0xe1510002 });  // cmp r1, r2
  // Should be true if no borrow occurred
  REQUIRE(static_test<systest.c_flag() == true>());
}

TEST_CASE("CMP with carry 3")
{
  constexpr auto systest = run_instruction(arm_thing::Instruction{ 0xe3a01000 },   // mov r1, #0
                                           arm_thing::Instruction{ 0xe3a02001 },   // mov r2, #1
                                           arm_thing::Instruction{ 0xe1510002 });  // cmp r1, r2
  // Should be false if a borrow occurred
  REQUIRE(static_test<systest.c_flag() == false>());
}


TEST_CASE("test add of register")
{
  constexpr auto systest4 = run_instruction(arm_thing::Instruction{ 0xe2800055 });  // add r0, r0, #85
  REQUIRE(static_test<systest4.registers[0] == 0x55>());
}

TEST_CASE("test add of register with shifts")
{
  constexpr auto systest5 = run_instruction(arm_thing::Instruction{ 0xe2800055 },  // add r0, r0, #85
                                            arm_thing::Instruction{ 0xe2800c7e }   // add r0, r0, #32256
  );
  REQUIRE(static_test<systest5.registers[0] == (85 + 32256)>());
}

TEST_CASE("test multiple adds and sub")
{
  constexpr auto systest6 = run_instruction(arm_thing::Instruction{ 0xe2800001 },  // add r0, r0, #1
                                            arm_thing::Instruction{ 0xe2811009 },  // add r1, r1, #9
                                            arm_thing::Instruction{ 0xe2822002 },  // add r2, r2, #2
                                            arm_thing::Instruction{ 0xe0423001 }   // sub r3, r2, r1
  );
  REQUIRE(static_test<systest6.registers[3] == static_cast<std::uint32_t>(2 - 9)>());
}

TEST_CASE("test add over 16bits")
{
  constexpr auto systest = run_instruction(arm_thing::Instruction{ 0xe3a010ff },  // mov r1, #255
                                           arm_thing::Instruction{ 0xe3811cff },   // orr r1, r1 #65280
                                           arm_thing::Instruction{ 0xe2811001 }   // add r1, r1, #1

  );
  REQUIRE(static_test<systest.registers[1] == 0x10000>());
}


TEST_CASE("test memory writes")
{
  constexpr auto systest6 = run_instruction(arm_thing::Instruction{ 0xe3a00064 },  // mov r0, #100 ; 0x64
                                            arm_thing::Instruction{ 0xe3a01005 },  // mov r1, #5
                                            arm_thing::Instruction{ 0xe5c01000 },  // strb r1, [r0]
                                            arm_thing::Instruction{ 0xe3a00000 },  // mov r0, #0
                                            arm_thing::Instruction{ 0xe1a0f00e }   // mov pc, lr
  );

  REQUIRE(static_test<systest6.read_byte(100) == 5>());
}


TEST_CASE("test lsr")
{
  constexpr auto sys = run_instruction(arm_thing::Instruction{ 0xe3a03005 },  // mov r3, #5
                                       arm_thing::Instruction{ 0xe1a02123 }   // lsr r2, r3, #2
  );
  REQUIRE(static_test<sys.registers[2] == 1>());
  REQUIRE(static_test<sys.registers[3] == 5>());
}

TEST_CASE("Test sub instruction with shift")
{
  constexpr auto systest7 = run_instruction(arm_thing::Instruction{ 0xe2800001 },  // add r0, r0, #1
                                            arm_thing::Instruction{ 0xe2811009 },  // add r1, r1, #9
                                            arm_thing::Instruction{ 0xe2822002 },  // add r2, r2, #2
                                            arm_thing::Instruction{ 0xe0403231 }   // sub r3, r0, r1, lsr r2
                                                                                   // logical right shift r1 by the number in bottom byte of r2
                                                                                   // subtract result from r0 and put answer in r3
  );

  REQUIRE(static_test<systest7.registers[3] == static_cast<std::uint32_t>(1 - (9 >> 2))>());
}

TEST_CASE("Test arbitrary code execution with loop")
{
  //#include <cstdint>

  // template<typename T>
  //  volatile T& get_loc(const std::uint32_t loc)
  //  {
  //    return *reinterpret_cast<T*>(loc);
  //  }

  // int main()
  //{
  //  for (int i = 0; i < 100; ++i) {
  //    get_loc<std::int8_t>(100+i) = i%5;
  //  }
  //}

  // compiles to:

  // 00000000 <main>:
  // 0:  e59f102c ldr	r1, [pc, #44]	; 34 <main+0x34>
  // 4:  e3a00000 mov	r0, #0
  // 8:  e0832190 umull	r2, r3, r0, r1
  // c:  e1a02123 lsr	r2, r3, #2
  // 10: e0822102 add	r2, r2, r2, lsl #2
  // 14: e2622000 rsb	r2, r2, #0
  // 18: e0802002 add	r2, r0, r2
  // 1c: e5c02064 strb	r2, [r0, #100]	; 0x64
  // 20: e2800001 add	r0, r0, #1
  // 24: e3500064 cmp	r0, #100	; 0x64
  // 28: 1afffff6 bne	8 <main+0x8>
  // 2c: e3a00000 mov	r0, #0
  // 30: e1a0f00e mov	pc, lr
  // 34: cccccccd .word	0xcccccccd

  constexpr std::array<std::uint8_t, 1024> memory{ 0x2c, 0x10, 0x9f, 0xe5, 0x00, 0x00, 0xa0, 0xe3, 0x90, 0x21, 0x83, 0xe0, 0x23, 0x21,
                                                   0xa0, 0xe1, 0x02, 0x21, 0x82, 0xe0, 0x00, 0x20, 0x62, 0xe2, 0x02, 0x20, 0x80, 0xe0,
                                                   0x64, 0x20, 0xc0, 0xe5, 0x01, 0x00, 0x80, 0xe2, 0x64, 0x00, 0x50, 0xe3, 0xf6, 0xff,
                                                   0xff, 0x1a, 0x00, 0x00, 0xa0, 0xe3, 0x0e, 0xf0, 0xa0, 0xe1, 0xcd, 0xcc, 0xcc, 0xcc };

  constexpr auto system = run_code(0, memory);

  REQUIRE(static_test<system.read_byte(100) == 0>());
  REQUIRE(static_test<system.read_byte(104) == 4>());
  REQUIRE(static_test<system.read_byte(105) == 0>());
  REQUIRE(static_test<system.read_byte(106) == 1>());
}


TEST_CASE("Test condition parsing")
{
  REQUIRE(static_test<arm_thing::Instruction{ 0b1110'1010'0000'0000'0000'0000'0000'1111 }.get_condition() == arm_thing::Condition::AL>());
}

TEST_CASE("Test mov parsing")
{
  // 0:	e3a000e9 	mov	r0, #233	; 0xe9

  constexpr arm_thing::Instruction ins{ 0b1110'0011'1010'0000'0000'0000'1110'1001 };
  constexpr arm_thing::Data_Processing dp{ ins };

  REQUIRE(static_test<ins.get_condition() == arm_thing::Condition::AL>());
  REQUIRE(static_test<dp.get_opcode() == arm_thing::OpCode::MOV>());
  REQUIRE(static_test<ins.unconditional()>());
  REQUIRE(static_test<arm_thing::System<>::decode(ins) == arm_thing::Instruction_Type::Data_Processing>());

  REQUIRE(static_test<dp.operand_1_register() == 0>());
  REQUIRE(static_test<dp.destination_register() == 0>());

  REQUIRE(static_test<dp.immediate_operand()>());
  REQUIRE(static_test<dp.operand_2_immediate() == 233>());
}

TEST_CASE("Test orr parsing")
{
  // e3800c03 	orr	r0, r0, #768	; 0x300

  constexpr arm_thing::Instruction ins{ 0b1110'0011'1000'0000'0000'1100'0000'0011 };
  constexpr arm_thing::Data_Processing dp{ ins };

  REQUIRE(static_test<ins.get_condition() == arm_thing::Condition::AL>());
  REQUIRE(static_test<ins.unconditional()>());
  REQUIRE(static_test<arm_thing::System<>::decode(ins) == arm_thing::Instruction_Type::Data_Processing>());

  REQUIRE(static_test<dp.get_opcode() == arm_thing::OpCode::ORR>());
  REQUIRE(static_test<dp.operand_1_register() == 0>());
  REQUIRE(static_test<dp.destination_register() == 0>());

  REQUIRE(static_test<dp.immediate_operand()>());
  REQUIRE(static_test<dp.operand_2_immediate() == 768>());
}

TEST_CASE("Test complex register value setting")
{
  // 0:	e3a000e9 	mov	r0, #233	; 0xe9
  // 4:	e3800c03 	orr	r0, r0, #768	; 0x300
  constexpr auto thing = run(0xe9, 0x00, 0xa0, 0xe3, 0x03, 0x0c, 0x80, 0xe3);
  //  std::cout << thing.registers[0] << '\n';
  REQUIRE(static_test<thing.registers[0] == 1001>());
}

TEST_CASE("Test arbitrary movs")
{
  // 0:	e3a000e9 	mov	r0, #233	; 0xe9
  // 4:	e3a0100c 	mov	r1, #12

  constexpr auto system = run(0xe9, 0x00, 0xa0, 0xe3, 0x0c, 0x10, 0xa0, 0xe3);
  REQUIRE(static_test<system.registers[0] == 233>());
  REQUIRE(static_test<system.registers[1] == 12>());
}

TEST_CASE("Test arbitrary code")
{

  // 00000000 <main>:
  // 0:	e3a000e9 	mov	r0, #233	; 0xe9
  // 4:	e3a0100c 	mov	r1, #12
  // 8:	e3800c03 	orr	r0, r0, #768	; 0x300
  // c:	e5c01000 	strb	r1, [r0]
  // 10:	e3a00000 	mov	r0, #0
  // 14:	e1a0f00e 	mov	pc, lr

  constexpr auto thing =
    run(0xe9, 0, 0xa0, 0xe3, 0x0c, 0x10, 0xa0, 0xe3, 0x03, 0x0c, 0x80, 0xe3, 0x00, 0x10, 0xc0, 0xe5, 0x00, 0x00, 0xa0, 0xe3, 0x0e, 0xf0, 0xa0, 0xe1);
  REQUIRE(static_test<thing.read_byte(1001) == 12>());
}
