#include <../../sim86_lib.cpp>
#include <ruby.h>

ID id_address;
ID id_size;
ID id_op;
ID id_ops[Op_Count];
ID id_lock;
ID id_rep;
ID id_segment;
ID id_wide;
ID id_far;
ID id_o1;
ID id_o2;
ID id_segment_override;
ID id_register;
ID id_offset;
ID id_count;
ID id_scale;
ID id_displacement;
ID id_explicit_segment;
ID id_t0;
ID id_t1;

#include <generated.inc>

static op2sym(operation_type op)
{
  if (op > Op_Count) { return Qnil; }
  return ID2SYM(id_ops[op]);
}

static VALUE register_to_obj(register_access reg)
{
  VALUE hash = rb_hash_new();
  rb_hash_aset(hash, ID2SYM(id_register), INT2NUM(reg.Index));
  rb_hash_aset(hash, ID2SYM(id_offset), INT2NUM(reg.Offset));
  rb_hash_aset(hash, ID2SYM(id_count), INT2NUM(reg.Count));
  return hash;
}

static VALUE address_term_to_obj(effective_address_term term)
{
  VALUE hash = register_to_obj(term.Register);
  rb_hash_aset(hash, ID2SYM(id_scale), INT2NUM(term.Scale));
  return hash;
}

static VALUE address_expr_to_obj(effective_address_expression expr)
{
  VALUE hash = rb_hash_new();
  rb_hash_aset(hash, ID2SYM(id_t0), address_term_to_obj(expr.Terms[0]));
  rb_hash_aset(hash, ID2SYM(id_t1), address_term_to_obj(expr.Terms[1]));
  if (expr.Flags & Address_ExplicitSegment)
  {
    rb_hash_aset(hash, ID2SYM(id_explicit_segment), INT2NUM(expr.ExplicitSegment));
  }
  if (expr.Displacement)
  {
    rb_hash_aset(hash, ID2SYM(id_displacement), INT2NUM(expr.Displacement));
  }
  return hash;
}

static VALUE operand2obj(instruction_operand od)
{
  switch (od.Type)
  {
  case Operand_Register: return register_to_obj(od.Register);
  case Operand_Memory: return address_expr_to_obj(od.Address);
  // Note: I don't think we care about checking the "RelativeJumpDisplacement"
  // flag.  That should always be obvious from the opcode.
  case Operand_Immediate: return INT2NUM(od.Immediate.Value);
  }
  return Qnil;
}

static VALUE sim86_decode(int argc, VALUE *argv, VALUE obj)
{
  if (argc < 1 || argc > 2)
  {
     rb_raise(rb_eArgError, "wrong number of arguments");
  }
  char * p = StringValuePtr(argv[0]);
  int length = RSTRING_LEN(argv[0]);
  int offset = argc >= 2 ? NUM2INT(argv[1]) : 0;
  if (offset < 0 || offset > length) { return Qnil; }
  p += offset;
  length -= offset;
  if (length == 0) { return Qnil; }

  instruction inst;
  Sim86_Decode8086Instruction(length, (unsigned char *)p, &inst);

  VALUE hash = rb_hash_new();

  if (argc >= 2)
  {
    rb_hash_aset(hash, ID2SYM(id_address), INT2NUM(offset + inst.Address));
    rb_hash_aset(hash, ID2SYM(id_size), INT2NUM(inst.Size));
  }

  rb_hash_aset(hash, ID2SYM(id_op), op2sym(inst.Op));

  if (inst.Flags & Inst_Lock) { rb_hash_aset(hash, ID2SYM(id_lock), Qtrue); }
  if (inst.Flags & Inst_Rep) { rb_hash_aset(hash, ID2SYM(id_rep), Qtrue); }
  if (inst.Flags & Inst_Segment) { rb_hash_aset(hash, ID2SYM(id_segment), Qtrue); }
  if (inst.Flags & Inst_Wide) { rb_hash_aset(hash, ID2SYM(id_wide), Qtrue); }
  if (inst.Flags & Inst_Far) { rb_hash_aset(hash, ID2SYM(id_far), Qtrue); }

  if (inst.Operands[0].Type)
  {
    rb_hash_aset(hash, ID2SYM(id_o1), operand2obj(inst.Operands[0]));
  }

  if (inst.Operands[1].Type)
  {
    rb_hash_aset(hash, ID2SYM(id_o2), operand2obj(inst.Operands[1]));
  }

  if (inst.SegmentOverride)
  {
    rb_hash_aset(hash, ID2SYM(id_segment_override), INT2NUM(inst.SegmentOverride));
  }

  return hash;
}

extern "C" void Init_sim86()
{
  init_ids();

  id_ops[Op_mov] = rb_intern("mov");

  VALUE sim86_mod = rb_define_module("Sim86");
  rb_define_const(sim86_mod, "VERSION", INT2FIX(SIM86_VERSION));

  rb_define_singleton_method(sim86_mod, "decode_8086_instruction", sim86_decode, -1);
  //VALUE sim86_class = rb_define_class("Sim86", rb_cObject);
  //rb_define_method(sim86_class, "initialize", sim86_inititialize, 0);
  // rb_define_method(sim86_class, "add", t_add, 1);
}
