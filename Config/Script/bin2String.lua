--exampple
-- BinToString("00100001010001110100111001001001010100100100100101000110")

function bin2String (value)
	leng = string.len(value)
	out = {}
	for c=1,leng/8 do
		local vx = string.sub(value,((c-1)*8)+1,((c-1)*8)+8)
		table.insert(out,1, string.char(tonumber(vx, 2)))
		--print(string.char(tonumber(vx, 2)))
	end
	return table.concat(out, "")
end