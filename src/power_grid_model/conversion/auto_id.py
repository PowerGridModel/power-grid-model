# SPDX-FileCopyrightText: 2022 Contributors to the Power Grid Model project <dynamic.grid.calculation@alliander.com>
#
# SPDX-License-Identifier: MPL-2.0

from typing import Any, Dict, Hashable, Optional


class AutoID:
    """
    Automatic ID generator class

    1. Usage without items:
        auto_id = AutoID()
        a = auto_id()  # a = 0
        b = auto_id()  # b = 1
        c = auto_id()  # c = 2
        item = auto_id[2]          # item = 2

    2. Usage with hashable items:
        auto_id = AutoID()
        a = auto_id(item="Alpha")   # a = 0
        b = auto_id(item="Bravo")   # b = 1
        c = auto_id(item="Alpha")   # c = 0 (because key "Alpha" already existed)
        item = auto_id[1]           # item = "Bravo"

    3. Usage with non-hashable items:
        auto_id = AutoID()
        a = auto_id(item={"name": "Alpha"}, key="Alpha")  # a = 0
        b = auto_id(item={"name": "Bravo"}, key="Bravo")  # b = 1
        c = auto_id(item={"name": "Alpha"}, key="Alpha")  # c = 0 (because key "Alpha" already existed)
        item = auto_id[1]                                 # item = {"name": "Alpha"}

    Note: clashing keys will update the item:
        auto_id = AutoID()
        a = auto_id(item={"name": "Alpha"},  key="Alpha")  # a = 0
        b = auto_id(item={"name": "Bravo"},  key="Bravo")  # b = 1
        c = auto_id(item={"name": "Charly"}, key="Alpha")  # c = 0 (because key "Alpha" already existed)
        item = auto_id[0]                                  # item = {"name": "Charly"}

    """

    def __init__(self):
        self._keys: Dict[Hashable, int] = {}
        self._items: List[Any] = []

    def __call__(self, item: Optional[Any] = None, key: Optional[Hashable] = None):
        """
        Generate a new unique numerical id for the item, or retrieve the previously generated id.
        If the item is not hashable (or not unique) a unique key should be supplied

        Args:
            item: The original item
            key: An optional key (if the item is not hashable or not unique)

        Returns:
            A unique numerical ID for the item/key

        """

        # If no item is supplied, just use the key, or use the same value as the numerical id
        if item is None:
            item = key if key is not None else len(self._keys)

            # If no key is supplied, use the item as a key
        if key is None:
            if item.__hash__ is None:
                raise TypeError(f"unhashable type: '{type(item).__name__}', "
                                "please supply a 'key' argument or use hashable items")
            key = item.__hash__()

        # If an ID was already generated for this key, return the previously generated id
        if key in self._keys:
            idx = self._keys[key]

            # Store the numeric id and let it refer to the key (id -> key)
            self._items[idx] = item

        else:
            # Otherwise, generate a new numeric id (0, 1, 2, 3, ...)
            idx = len(self._keys)

            # Store the key and let it refer to the numeric id (key -> id)
            self._keys[key] = idx

            # Add the item (id -> item)
            self._items.append(item)

        # Return the numeric id
        return idx

    def __getitem__(self, idx: int) -> Any:
        """
        Get the original item by numerical index

        Args:
            idx: unique numerical index

        Returns:
            The original item
        """
        return self._items[idx]
