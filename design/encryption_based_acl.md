Encryption Based ACL
====================

NACENT DOCUMENT

A basic ACL works by assigning a set of permissable actions to each user.

Basic actions are R, W, X

Abstractly, this can be expressed like:
	Action 1 is permitted to User 1
	Action 2 is permitted to User 1
	Action 2 is permitted to User 2

Each action could be a Heading.

	Action 1
	Action 2
	Action 3

The users permitted, are grouped under each action

	Action 1
		User 1
	Action 2
		User 1
		User 2

The users are assigned Action Groups

	Action Group 1
	Action Group 2

These action groups are universally unique.

Each Action Group is given a Crypto Key

	Action Group 1
		Key 1
	Action Group 2
		Key 2

The crypto keys are also universally unique.

A user would identify himself by providing the key.  The key would enable the action.  An inode would have an encrypted action list.  Each item in the list would be encrypted by one of the keys.

	Inode
		Encrypted Action 1
		Encrypted Action 2

If a given key can't decrypt one of the actions, the user is denied.

If the key decrypts an action we move forward with the action.

	If "read"
		data pointed to by inode is expected to be encrypted with the key.  We send the data back to the user, encrypted.

	If "write"
		data buffered for write to the inode is expected to be encrypted with the read key.
		the user must prove they also have read permission by providing the read key before the buffer.  This read-key is encrypted using the write key.
		now we can write the buffer to the data pointed to by the inode.

Adding users
	Adding users is as simple as sharing the action key.

Removing Users
	Removing "read" users.

	A new read-key is created.  A person with write access to a certain location is given the new-read-key.  The person uses the old read key to read the old location, uses the new-read-key to encrypt the data, as well as the read-permission token, uses the write key to write to the new location.  Uses the write-key to unlink the previous location and link in the new location.
	
	Then the new read-key is assigned to the new group.

	Removing "write" users.

	A new write-key is created.  The old write key is used to access the old write permission, the write permission is rekeyed.

	The new-write-key is assigned to the new group.  It is used to encrypt the read-key at the head of the next write-buffer.

Ownership
	There needs to be an owner key.  The owner is given permission to change permissions.  By encrypting the entire ACL using the owner key and using the encypted version as a control set, can verify whether the ACL has been tampered with.  Can require that the current ACL matches before proceeding with changes.


Key Management
	A user would need to correlate his keys with what files he has access to.  There's no sense throwing all your keys at one file.

	In another way, keys correspond to group-action pairs.

	Key 1
		Raygan only Read
	Key 2
		Nathan and Raygan Read
	Key 3
		Nathan and Raygan Write
	Key 4
		Nathan and Raygan Search/Execute

	My first action upon selecting a URL would be selecting a key to use, selected by Group and Action.


All of the Above are Wide open to Man in the Middle Attacks


