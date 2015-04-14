class MaruBatsu
  attr_accessor :board
  attr_reader   :turn

  WIDTH   = 3     # ボードの横幅
  HEIGHT  = 3     # ボードの縦幅
  NONE    = '-'   # 何も無いセル
  MARU    = 'o'   # 先手のシンボル
  BATXU   = 'x'   # 後手のシンボル

  def initialize
    @board = Array.new(HEIGHT){ Array.new(WIDTH, '-') }
    @turn = 0

    @aiA = IO.popen('ruby ai.rb', 'r+')
    @aiB = IO.popen('ruby sample.rb', 'r+')

    run
  end

  def win_message
    puts "Winner #{turn.odd? ? 'First Player' : 'Second Player'}"
  end

  def draw_message
    puts 'This game is draw'
  end

  def mark
    turn.odd? ? 'o' : 'x'
  end

  # ゲーム中のイテレーション
  def run
    [@aiA, @aiB].cycle.take(9).each do |ai|
      @turn += 1

      ai.puts(input)
      output = ai.gets.chomp.split(' ')

      output = output.map(&:to_i)

      put(output.first, output.last)

      if game_finish?
        win_message
        break
      elsif draw?
        draw_message
        break
      end
    end

    show_board
  end

  # AI向けの入力を出力する
  def input
    params = []
    
    params << turn

    board.each do |line|
      params << line.join
    end

    params.join("\n")
  end

  def diagonal_lines
    [
      [board[0][0],board[1][1],board[2][2]],
      [board[0][2],board[1][1],board[2][0]]
    ]
  end

  def vertical_lines
    board.transpose
  end

  def horizontal_lines
    board
  end

  # ゲームが終了したかどうかを確認
  # 条件1: どちらかが勝利
  # 条件2: もう置ける場所が無い
  def game_finish?
    horizontal_lines.each do |line|
      return true if line.all?{|sym| sym == mark}
    end

    vertical_lines.each do |line|
      return true if line.all?{|sym| sym == mark}
    end

    diagonal_lines.each do |line|
      return true if line.all?{|sym| sym == mark}
    end

    false
  end

  def draw?
    turn == 9
  end

  # 何もシンボルが置かれていなければ置くことが出来る
  def puttable?(y, x)
    board[y][x] == NONE
  end

  # シンボルの設置を行う
  def put(width, height)
    board[width][height] = mark
  end

  # ボードの状態を表示する
  def show_board
    board.each do |line|
      $stderr.puts line.join
    end
  end
end
