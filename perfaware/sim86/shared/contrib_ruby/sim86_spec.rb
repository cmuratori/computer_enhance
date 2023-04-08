# coding: ASCII-8BIT

require './sim86'

describe Sim86 do
  specify 'version' do
    expect(Sim86::VERSION).to eq 3
  end

  describe 'decode_8086_instruction' do
    specify 'invalid argument types' do
      expect { Sim86::decode_8086_instruction(0, 0) }.to raise_error TypeError
      expect { Sim86::decode_8086_instruction(0, "") }.to raise_error TypeError
    end

    specify 'empty input' do
      expect(Sim86::decode_8086_instruction("")).to eq nil
    end

    specify 'invalid offsets' do
      expect(Sim86::decode_8086_instruction(".", -1)).to eq nil
      expect(Sim86::decode_8086_instruction(".", 1)).to eq nil
      expect(Sim86::decode_8086_instruction(".", 2)).to eq nil
    end

    specify 'some moves' do
      # mov si, bx
      # mov dh, al
      moves = "\x89\xde\x88\xc6"
      expect(Sim86::decode_8086_instruction(moves)).to eq ({
        op: :mov,
        wide: true,
        o1: {register: 7, offset: 0, count: 2 },
        o2: {register: 2, offset: 0, count: 2 },
      })
      expect(Sim86::decode_8086_instruction(moves, 2)).to eq ({
        op: :mov,
        o1: {register: 4, offset: 1, count: 1 },
        o2: {register: 1, offset: 0, count: 1 },
        address: 2, size: 2,
      })
    end

    specify 'immediates' do
      # mov bl, 44
      expect(Sim86::decode_8086_instruction("\xb3\x2c")).to eq ({
        op: :mov,
        o1: {register: 2, offset: 0, count: 1 },
        o2: 44,
      })

      # mov cx, 12
      expect(Sim86::decode_8086_instruction("\xb9\x0c\x00")).to eq ({
        op: :mov,
        wide: true,
        o1: { register: 3, offset: 0, count: 2 },
        o2: 12,
      })

      # mov dx, -3948
      expect(Sim86::decode_8086_instruction("\xba\x94\xf0")).to eq ({
        op: :mov,
        wide: true,
        o1: { register: 4, offset: 0, count: 2 },
        o2: -3948 & 0xFFFF,
      })
    end

    specify 'address expression' do
      # mov al, [bx + si + 4999]
      expect(Sim86::decode_8086_instruction("\x8a\x80\x87\x13")).to eq ({
        op: :mov,
        o1: { register: 1, count: 1, offset: 0 },
        o2: {
          t0: { register: 2, offset: 0, count: 2, scale: 1 },
          t1: { register: 7, offset: 0, count: 2, scale: 1 },
          displacement: 4999,
        },
      })
    end
  end
end
