class AI
  attr_accessor :board

  def initialize
    run
  end

  def run
    loop do
      turn = gets

      break if turn.nil?

      turn = turn.chomp.to_i

      @board = []

      3.times do
        line = gets.chomp

        @board << line
      end

      output = solve
      puts output.join(' ')
      $stdout.flush

      break if turn >= 8
    end
  end

  def solve
    loop do
      num = rand(9)

      y = num / 3
      x = num % 3

      if valid_output?(y, x)
        return [y, x]
      end
    end
  end

  def valid_output?(y, x)
    board[y][x] == '-'
  end
end

AI.new
