
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity clock_divider is
Generic (
--00000		
clkTime: POSITIVE := 50000000;
dutyPercentage_25: POSITIVE := 12500000;
dutyPercentage_75: POSITIVE := 37500000
);
port (
 clk: IN STD_LOGIC;
 Speed: IN STD_LOGIC;
 dutyOff: out std_logic;
 dutyOn: out std_logic;
 one_second: out std_logic
);
end clock_divider;

architecture implementation of clock_divider is
begin
PROCESS (clk)
variable count: integer range 0 to clkTime + 1;
begin
IF (RISING_EDGE(clk)) THEN
	count := count + 1;
	IF (Speed = '0') then
		if (count = clkTime) then
			count := 0;
			one_second <= '1';
		else if(count <= dutyPercentage_25) then
			dutyOn <= '1';
			dutyOff <= '0';
			one_second <= '0';
		else			--na 250
			dutyOff <= '1';
			dutyOn <= '0';
			one_second <= '0';
		end if;
		end if;		
	else
		if(count <= dutyPercentage_75) then
			dutyOn <= '1';
			dutyOff <= '0';
			one_second <= '0';
		else			--na 750
			dutyOff <= '1';
			dutyOn <= '0';
			one_second <= '0';
		end if;
		if (count = clkTime) then
			count := 0;
			one_second <= '1';
		end if;
	end if;
end if;
END PROCESS;
end implementation;