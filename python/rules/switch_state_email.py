
def on_living_room_light_change(location, name, value):
	print location,"-",name,"(" + str(value) + ")"

if 'rulemap' not in globals():
	raise NameError("'rulemap' was not defined")

rulemap[2].append(on_living_room_light_change)
