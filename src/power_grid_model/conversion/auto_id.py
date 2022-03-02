from typing import Dict, Hashable


class AutoID:

    def __init__(self):
        self.items: Dict[Hashable, int] = {}

    def __getitem__(self, item: Hashable):
        if item in self.items:
            item_id = self.items[item]
        else:
            item_id = len(self.items) + 1
            self.items[item] = item_id
        return item_id
