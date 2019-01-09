import json

with open('AllSets.json', 'r') as infile, open('AllSetsFixed.json', 'w') as outfile:
	allsets = json.load(infile)
	for setName, set in allsets.items():
		for card in set['cards']:
			if 'foreignData' in card:
				del card['foreignData']
	json.dump(allsets, outfile)
