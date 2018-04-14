import argparse

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

tradeNodeConfig = Config.readFromFile(tradeNodeFilePath)





