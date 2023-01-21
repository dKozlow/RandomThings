LIBRARY ieee;
USE ieee.std_logic_1164.all;
USE ieee.numeric_std.ALL;
ENTITY component_tutorial IS
    GENERIC (
        clkTime : POSITIVE := 50000000;
        data_width : POSITIVE := 40);
	PORT (
		CLOCK_50 : IN STD_LOGIC;
		KEY : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
		SW : IN STD_LOGIC_VECTOR (9 DOWNTO 0);
		dht22 : INOUT STD_LOGIC;
		LEDR : OUT STD_LOGIC_VECTOR (9 DOWNTO 0);
		HEX0 : OUT STD_LOGIC_VECTOR(6 DOWNTO 0);
		HEX1 : OUT STD_LOGIC_VECTOR(6 DOWNTO 0);
		HEX2 : OUT STD_LOGIC_VECTOR(6 DOWNTO 0);
		HEX3 : OUT STD_LOGIC_VECTOR(6 DOWNTO 0));
END component_tutorial;

ARCHITECTURE Structure OF component_tutorial IS

COMPONENT embedded_system IS
        port (
            clk_clk            : in  std_logic                     := 'X';             -- clk
            humidity_export    : in  std_logic_vector(11 downto 0) := (others => 'X'); -- export
            leds_export        : out std_logic_vector(9 downto 0);                     -- export
            reset_reset_n      : in  std_logic                     := 'X';             -- reset_n
            switches_export    : in  std_logic_vector(9 downto 0)  := (others => 'X'); -- export
            temperature_export : in  std_logic_vector(11 downto 0) := (others => 'X'); -- export
            --to_hex_readdata    : out std_logic_vector(31 downto 0)                     -- readdata
				hex01_export       : out std_logic_vector(6 downto 0);
				hex02_export       : out std_logic_vector(6 downto 0);
				hex03_export       : out std_logic_vector(6 downto 0);
				hex04_export       : out std_logic_vector(6 downto 0)
        );
END COMPONENT embedded_system;
COMPONENT hex7seg IS
PORT ( 	hex : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
			display : OUT STD_LOGIC_VECTOR(0 TO 6) );
END COMPONENT hex7seg;

    COMPONENT clock_divider IS
        PORT (
            clk : IN STD_LOGIC;
            Speed : IN STD_LOGIC;
            dutyOff : OUT STD_LOGIC;
            dutyOn : OUT STD_LOGIC;
            one_second : OUT STD_LOGIC
        );
    END COMPONENT;
    CONSTANT delay_1_us : POSITIVE := clkTime/1000000;
    CONSTANT delay_28_us : POSITIVE := delay_1_us * 28 + 1;
    CONSTANT delay_35_us : POSITIVE := delay_1_us * 35 + 1;
    CONSTANT delay_80_us : POSITIVE := delay_1_us * 80 + 1;
    CONSTANT delay_500_us : POSITIVE := delay_1_us * 500 + 1;
    CONSTANT delay_1_s : POSITIVE := clkTime + 1;
    CONSTANT max_delay : POSITIVE := delay_1_s;
	  CONSTANT delay_18_ms : POSITIVE := delay_1_us * (500*36) + 1;

    -- allstates
    TYPE state_type IS (initial_80_0, initial_80_1, initial_ack_0, initial_ack_1, wait_for_data, recieved_data, done);
    SIGNAL state : state_type := initial_80_0;
    SIGNAL data : STD_LOGIC_VECTOR(0 TO data_width - 1); -- 40 bits
    SIGNAL done_receiving : STD_LOGIC;
    SIGNAL output_enable : STD_LOGIC;
    SIGNAL input_dht_bus : STD_LOGIC;
    SIGNAL data_high_low : STD_LOGIC;
    SIGNAL display_temperature : STD_LOGIC_VECTOR(0 TO 11);
    SIGNAL display_humidity : STD_LOGIC_VECTOR(0 TO 11);
    SIGNAL led_speed : STD_LOGIC;
    SIGNAL led_on_off : STD_LOGIC;

    SIGNAL dutyLow : STD_LOGIC;
    SIGNAL dutyHigh : STD_LOGIC;
	 
	 
signal hex01 : STD_LOGIC_VECTOR(6 DOWNTO 0);
signal hex02 : STD_LOGIC_VECTOR(6 DOWNTO 0);
signal hex03 : STD_LOGIC_VECTOR(6 DOWNTO 0);
signal hex04 : STD_LOGIC_VECTOR(6 DOWNTO 0);
	 
BEGIN
 
	
	sync_flop : PROCESS (CLOCK_50) IS
    BEGIN
        IF rising_edge (CLOCK_50) THEN
            input_dht_bus <= dht22;
        END IF;
    END PROCESS sync_flop;
 pr_next_state : PROCESS (state, CLOCK_50)
        VARIABLE count : INTEGER RANGE 0 TO max_delay + 1 := 0;
        VARIABLE nBit : INTEGER RANGE 0 TO data_width := 0;
    BEGIN
        IF (RISING_EDGE(CLOCK_50)) THEN
            CASE(state) IS
                -- Start condition
                WHEN initial_80_0 => -- initial
                nBit := 0;
                count := count + 1;
                IF (count <= delay_18_ms) THEN -- 500us delay as start signal
                    output_enable <= '1';
                    data_high_low <= '0';
                ELSE
                    state <= initial_80_1;
                    count := 0;
                END IF;

                WHEN initial_80_1 =>
                count := count + 1;
                IF (count <= delay_28_us) THEN
                    output_enable <= '0';
                    data_high_low <= '1';
                ELSE
                    state <= initial_ack_0;
                    output_enable <= '1';
                    data_high_low <= '0';
                    count := 0;
                END IF;

                -- Acknowledge
                WHEN initial_ack_0 => -- wait response signal from dht22 should be 80 us low
                output_enable <= '0';
                IF (input_dht_bus = '1') THEN
                    state <= initial_ack_1;
                ELSE
                    state <= initial_ack_0;
                END IF;
                WHEN initial_ack_1 => -- wait response signal from dht22 should be 80 us high
                output_enable <= '0';
                IF (input_dht_bus = '0') THEN
                    state <= wait_for_data;
                ELSE
                    state <= initial_ack_1;
                END IF;

                WHEN wait_for_data => -- wait for data 50 us between each bit
                output_enable <= '0';
                IF (nBit = data_width) THEN
                    state <= done;
                    done_receiving <= '1';
                    count := 0;
                ELSE
                    IF (input_dht_bus = '1') THEN
                        done_receiving <= '0';
                        state <= recieved_data;
                        count := 0;
                    ELSE
                        state <= wait_for_data;
                        done_receiving <= '0';
                    END IF;
                END IF;
                WHEN recieved_data =>
                output_enable <= '0';
                count := count + 1;
                IF (input_dht_bus = '0') THEN
                    IF (count <= delay_35_us) THEN
                        data(nBit) <= '0';
                        nBit := nBit + 1;
                        state <= wait_for_data;
                    ELSE
                        data(nBit) <= '1';
                        nBit := nBit + 1;
                        state <= wait_for_data;
                    END IF;
                    count := 0;
                ELSE
                    state <= recieved_data;
                END IF;
                WHEN done =>
                count := count + 1;
                done_receiving <= '0';
                IF (count < delay_1_s) THEN
                    output_enable <= '1';
                    data_high_low <= '1';
                ELSE
                    state <= initial_80_0;
                    count := 0;
                    nBit := 0;
                END IF;
                WHEN OTHERS => -- others
                count := 0;
                nBit := 0;
                state <= initial_80_0;
            END CASE;
        END IF;
    END PROCESS;
    dht22 <= data_high_low WHEN output_enable = '1' ELSE
        'Z'; -- output enable is high then output data_high_low else high impedance

    temperature_process : PROCESS (done_receiving)
        VARIABLE temperature : STD_LOGIC_VECTOR(0 TO 11);
        VARIABLE humidity : STD_LOGIC_VECTOR(0 TO 11);
        VARIABLE integer_temperature : INTEGER RANGE 0 TO 1000;
        VARIABLE integer_humidity : INTEGER RANGE 0 TO 1000;
    BEGIN
        IF done_receiving = '1' THEN
            humidity := "00" & data(6 TO 15);
            temperature := "00" & data(22 TO 31);
            integer_temperature := to_integer(unsigned(temperature));
            integer_humidity := to_integer(unsigned(humidity));
            IF (integer_temperature > 260 OR integer_humidity > 700) THEN
                led_speed <= '1';
                led_on_off <= '1';
            ELSE
                led_on_off <= '0';
            END IF;
        END IF;
        display_temperature <= temperature;
        display_humidity <= humidity;
    END PROCESS;
	clk_divider : clock_divider PORT MAP(CLOCK_50, led_speed, dutyLow, dutyHigh);
   u0 : component embedded_system
        port map (
            clk_clk            => CLOCK_50,            --         clk.clk
            humidity_export    => display_humidity,    --    humidity.export
            leds_export        => LEDR(9 DOWNTO 0),        --        leds.export
            reset_reset_n      => KEY(0),      --       reset.reset_n
            switches_export    => SW(9 DOWNTO 0),    --    switches.export
            temperature_export => display_temperature, -- temperature.export
            --to_hex_readdata    => to_HEX     --      to_hex.readdata
				hex01_export       => HEX0 (6 DOWNTO 0),
				hex02_export       => HEX1 (6 DOWNTO 0),
				hex03_export       => HEX2 (6 DOWNTO 0),
				hex04_export       => HEX3 (6 DOWNTO 0)
        );

END Structure;

