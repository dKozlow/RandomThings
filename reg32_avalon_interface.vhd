LIBRARY ieee;
USE ieee.std_logic_1164.all;
ENTITY reg32_avalon_interface IS
PORT ( clock, resetn : IN STD_LOGIC;
	read, write, chipselect : IN STD_LOGIC;
	writedata : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
	byteenable : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
	readdata : OUT STD_LOGIC_VECTOR(31 DOWNTO 0);
	Q_export : OUT STD_LOGIC_VECTOR(41 DOWNTO 0);
	address : IN STD_LOGIC
);
END reg32_avalon_interface;

ARCHITECTURE Structure OF reg32_avalon_interface IS
	SIGNAL local_byteenable, local_byteenable2 : STD_LOGIC_VECTOR(3 DOWNTO 0);
	SIGNAL to_reg, from_reg : STD_LOGIC_VECTOR(31 DOWNTO 0);
	SIGNAL to_reg2, from_reg2 : STD_LOGIC_VECTOR(31 DOWNTO 0);
	SIGNAL temp : STD_LOGIC_VECTOR(41 DOWNTO 0);
	signal tmp: STD_LOGIC_VECTOR(1 DOWNTO 0);
COMPONENT reg32
PORT (
	clock, resetn : IN STD_LOGIC;
	D : IN STD_LOGIC_VECTOR(31 DOWNTO 0);
	byteenable : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
	Q : OUT STD_LOGIC_VECTOR(41 DOWNTO 0) );
END COMPONENT;

COMPONENT hex7seg IS
		PORT ( hex : IN STD_LOGIC_VECTOR(3 DOWNTO 0);
			display : OUT STD_LOGIC_VECTOR(0 TO 6) );
		END COMPONENT hex7seg;

BEGIN
	tmp(1) <= chipselect AND write;
	tmp(0) <= address;

	WITH tmp SELECT
		to_reg <= writedata WHEN "11", from_reg when others;

	WITH tmp SELECT
		to_reg2 <= writedata WHEN "10", from_reg2 when others;
		
	WITH (tmp) SELECT
		local_byteenable <= byteenable WHEN "11", "0000" WHEN OTHERS; 

	WITH (tmp) SELECT
		local_byteenable2 <= byteenable WHEN "10", "0000" WHEN OTHERS; 
		
	reg_instance: reg32 PORT MAP (clock, resetn, to_reg, local_byteenable, from_reg);
	reg_instance2: reg32 PORT MAP (clock, resetn, to_reg2, local_byteenable2, from_reg2);

	h0: hex7seg PORT MAP (from_reg(3 DOWNTO 0), temp(6 downto 0));
	h1: hex7seg PORT MAP (from_reg(7 DOWNTO 4), temp(13 downto 7));
	h2: hex7seg PORT MAP (from_reg(11 DOWNTO 8), temp(20 downto 14));
	h3: hex7seg PORT MAP (from_reg(15 DOWNTO 12), temp(27 downto 21));
	h4: hex7seg PORT MAP (from_reg2(3 DOWNTO 0), temp(34 downto 28));
	h5: hex7seg PORT MAP (from_reg2(7 DOWNTO 4), temp(41 downTo 35));
	
	with address select
		readdata <= from_reg when '1', from_reg2 when others;
	
	Q_export <= temp;	
END Structure;
