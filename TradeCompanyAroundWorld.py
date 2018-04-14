import argparse
import random

def parse_args():
	parser = argparse.ArgumentParser(description='Generate modding file for trade companies on all of trade nodes.')
	parser.add_argument('path', help='Path to EU4 root directory.')
	parser.add_argument('--ICR', dest='icr', action='store_true', help='also make colonial regions to trade companies.')
	return parser.parse_args()

args = parse_args()
if not args.path.endswith('\\'):
	args.path += '\\'

# input
tradeNodeFilePath = args.path + 'common\\tradenodes\\00_tradenodes.txt'
colonialRegionFilePath = args.path + 'common\\colonial_regions\\00_colonial_regions.txt'

#output
tradeCompanyFilePath = 'output/00_trade_companies.txt'

from common.fileWrapper import EU4ConfigComposite as Config
from common.fileWrapper import EU4ConfigScope as Scope
from common.fileWrapper import EU4ConfigExpression as Expression

tradeNodeConfig = Config.readFromFile(tradeNodeFilePath)

tradeCompanyConfigs = Config()

for node in tradeNodeConfig.children:
	nodeName = node.key
	tradeCompanyName = 'trade_company_' + nodeName
	tradeCompanyConfig = Scope(tradeCompanyName)

	if node['color'] is not None:
		tradeCompanyConfig.appendScope(node['color'])
	else:
		colors = [str(random.randint(0, 255)) for _ in range(3)]
		tradeCompanyConfig.appendList('color', colors)

	provinces = node['members']
	provinces.key = 'provinces'
	tradeCompanyConfig.appendScope(provinces)
	
	tradeCompanyNames = Scope('names')
	tradeCompanyNames.appendExpression(('name', tradeCompanyName.upper() + '_Root_Culture_GetName'))
	tradeCompanyConfig.appendScope(tradeCompanyNames)

	tradeCompanyNames = Scope('names')
	tradeCompanyNames.appendExpression(('name', tradeCompanyName.upper() + '_Trade_Company'))
	tradeCompanyConfig.appendScope(tradeCompanyNames)

	tradeCompanyConfigs.appendScope(tradeCompanyConfig)

with open(tradeCompanyFilePath, 'w') as outfile:
	outfile.write(str(tradeCompanyConfigs))