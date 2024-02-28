// SPDX-License-Identifier: UNLICENSED

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/proc_fs.h>
// Fournit une interface pour l'écriture de fichiers séquentiels dans le noyau.
#include <linux/seq_file.h>
#include <linux/spinlock.h>
#include <linux/nsproxy.h>
#include <linux/ns_common.h>
#include <linux/poll.h>
#include <linux/mnt_namespace.h>
#include <../fs/mount.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("mberengu");
MODULE_DESCRIPTION("whatever!");

static struct proc_dir_entry *entry;

/* Simplied version of
 * https://elixir.bootlin.com/linux/latest/source/fs/proc_namespace.c#L101 */
static int procmnt_show(struct seq_file* m, void* v)
{
	// C'est une structure du noyau Linux représentant un point de montage.
	// Elle est définie dans <../fs/mount.h>
	struct      mount* mnt;

	// mnt      = variable qui représente un pointeur vers chaque élément de la liste
	// current  = tete de list sur lequel commence l iteration. pointer vers le "current" process
	// mnt_list = element crée par l'itération
	
	// pseudo python
	// for mnt in current->etc..:
	// 	mnt_list.append(mnt)
	list_for_each_entry(mnt, &current->nsproxy->mnt_ns->list, mnt_list)
	{
		//Cette structure représente un chemin dans le système de fichiers.s
		//  Elle contient un dentry (directory entry)
		//  et un mnt (point de montage
		struct path		mnt_path = { .dentry = mnt->mnt.mnt_root, .mnt = &mnt->mnt };

		// Représente un bloc super dans le système de fichiers
		// Un Super block contient des informations essentielles
		// sur un système de fichiers particulier,
		// telles que le type de système de fichiers, la taille des blocs,
		// le nombre total de blocs, le nombre de blocs libres, etc.
		struct super_block	*sb = mnt_path.dentry->d_sb;

		/* Skip first `mount`, that appears to be the VFS / itself */
		if (!strcmp(mnt->mnt_devname, "rootfs"))
			continue ;

		if (sb->s_op->show_devname)
			sb->s_op->show_devname(m, mnt_path.dentry);
		else
			seq_puts(m, mnt->mnt_devname ? mnt->mnt_devname : "none");
		// écris un caractère dans un fichier séquentiel
		seq_putc(m, ' ');
		// écris le chemin d'un objet de type struct path dans un fichier séquentiel
		seq_path(m, &mnt_path, " \t\n\\");
		seq_putc(m, '\n');
	}
	return 0;
}

static int procmnt_open(struct inode* inode, struct file* file)
{
	// Fonction d'assistance pour l'ouverture de fichiers séquentiels
	return single_open(file, procmnt_show, NULL);
}

// proc_ops: Structure qui définit les opérations associées à un fichier procfs
static const struct proc_ops procmnt_fops = {
	.proc_open = procmnt_open,
	// seq_read est une fonction de lecture générique utilisée pour
	// les fichiers séquentiels dans l'espace du noyau
	.proc_read = seq_read,
	// La fonction seq_lseek est une fonction de déplacement de fichier générique
	// utilisée pour les fichiers séquentiels dans l'espace du noyau.
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static int __init procmnt_init(void)
{
	// Crée un fichier virtuel dans le système de fichiers procfs
	
	// proc_create(<name>, <autorisations>, <structure parente> ,<struct proc_ops>);
	// name   = mymounts
	// 0      = autorisation par default (0644)
	// NULL   = creation dans fichier racine (proc)
	// struct = pour comportement
	entry = proc_create("mymounts", 0, NULL, &procmnt_fops);
	if (entry == NULL) {
		printk(KERN_ERR "procmnt: failed to create /proc/procmnt file\n");
		return -ENOMEM;
	}

	printk(KERN_INFO "procmnt: created\n");
	return 0;
}

static void __exit procmnt_exit(void)
{
	proc_remove(entry);
	printk(KERN_INFO "procmnt: removed\n");
}

module_init(procmnt_init);
module_exit(procmnt_exit);

// CONCLUSION le kernel est un systeme de structures imbriquée à connaitre pour faire le taf
// C'est framework genre unity ou unreal: un set de fonctions et structures existantes qui font le café

/*				   STRUCTS 

			-------- SUPER_BLOCK --------
struct super_block {
	struct list_head	s_list;i		// Liste de super_blocks
	dev_t			s_dev;			// Identifiant du périphérique
	unsigned char		s_blocksize_bits;	// Nombre de bits pour la taille de bloc
	unsigned long		s_blocksize;		// Taille de bloc
	loff_t			s_maxbytes;		// Taille maximale du fichier
	struct file_system_type *s_type;		// Type de système de fichiers
	const struct super_operations *s_op;		// Opérations sur le super_block
	const struct dquot_operations *dq_op;		// Opérations sur les quotas de disque
	const struct quotactl_ops *s_qcop;		// Opérations de contrôle de quota
	const struct export_operations *s_export_op;	// Opérations d'exportation
	unsigned long		s_flags;		// Drapeaux du super_block
	unsigned long		s_iflags;		// Drapeaux internes SB_I_*
	unsigned long		s_magic;		// Numéro magique
	struct dentry		*s_root;		// Racine du système de fichiers
	struct rw_semaphore	s_umount;		// Sémaphore de démontage
	int			s_count;		// Nombre de références
	atomic_t		s_active;		// Nombre d'utilisateurs actifs
#ifdef CONFIG_SECURITY
	void                    *s_security;		// Sécurité
#endif
	const struct xattr_handler * const *s_xattr;	// Gestionnaire d'attributs étendus
#ifdef CONFIG_FS_ENCRYPTION
	const struct fscrypt_operations *s_cop;		// Opérations de cryptage
	struct fscrypt_keyring	*s_master_keys;		// Clés de cryptage
#endif
#ifdef CONFIG_FS_VERITY
	const struct fsverity_operations *s_vop;	// Opérations de vérification
#endif
#if IS_ENABLED(CONFIG_UNICODE)
	struct unicode_map *s_encoding;			// Carte Unicode
	__u16 s_encoding_flags;				// Drapeaux d'encodage
#endif
	struct hlist_bl_head	s_roots;		// Entrées de la racine NFS
	struct list_head	s_mounts;		// Liste des montages
	struct block_device	*s_bdev;		// Périphérique de bloc associé
	struct bdev_handle	*s_bdev_handle;		// Poignée du périphérique de bloc
	struct backing_dev_info *s_bdi;			// Informations de sauvegarde
	struct mtd_info		*s_mtd;			// Informations MTD
	struct hlist_node	s_instances;		// Instances
	unsigned int		s_quota_types;		// Types de quota supportés
	struct quota_info	s_dquot;		// Options de quotas de disque
	struct sb_writers	s_writers;		// Écrivains du super_block
	void			*s_fs_info;		// Informations privées du système de fichiers
	u32			s_time_gran;		// Granularité temporelle
	time64_t		s_time_min;		// Temps minimal
	time64_t		s_time_max;		// Temps maximal
#ifdef CONFIG_FSNOTIFY
	__u32			s_fsnotify_mask;	// Masque de notification de système de fichiers
	struct fsnotify_mark_connector __rcu	*s_fsnotify_marks;	// Connecteurs de notification de système de fichiers
#endif
	char			s_id[32];		// Nom informatif
	uuid_t			s_uuid;			// UUID
	unsigned int		s_max_links;		// Nombre maximal de liens
	struct mutex		s_vfs_rename_mutex;	// Mutex de renommage VFS
	const char *s_subtype;				// Sous-type de système de fichiers
	const struct dentry_operations *s_d_op;		// Opérations par défaut pour les dentries
	struct shrinker *s_shrink;			// Gestionnaire de réduction
	atomic_long_t s_remove_count;			// Nombre d'inodes avec nlink == 0 mais toujours référencés
	atomic_long_t s_fsnotify_connectors;		// Nombre d'objets surveillés
	int s_readonly_remount;				// Remontage en lecture seule
	errseq_t s_wb_err;				// Gestionnaire d'erreur de writeback
	struct workqueue_struct *s_dio_done_wq;		// File d'attente pour les AIO complétés
	struct hlist_head s_pins;			// Pins
	struct user_namespace *s_user_ns;		// Espace de nom d'utilisateur
	struct list_lru	s_dentry_lru;			// LRU pour les dentries
	struct list_lru	s_inode_lru;			// LRU pour les inodes
	struct rcu_head	rcu;				// Tête RC
	struct work_struct	destroy_work;		// Travail de destruction
	struct mutex		s_sync_lock;		// Mutex de synchronisation
	int s_stack_depth;				// Profondeur de la pile
	spinlock_t		s_inode_list_lock;	// Mutex pour la liste des inodes
	struct list_head	s_inodes;		// Liste de tous les inodes
	spinlock_t		s_inode_wblist_lock;	// Mutex pour les inodes en writeback
	struct list_head	s_inodes_wb;		// Inodes en writeback
} __randomize_layout;				// Disposition aléatoire des membres


			-------- MOUNT --------
struct mount {
	struct hlist_node mnt_hash;			// Entrée de hachage pour la recherche rapide
	struct mount *mnt_parent;			// Pointeur vers le parent de ce point de montage
	struct dentry *mnt_mountpoint;			// Dentry du point de montage
	struct vfsmount mnt;				// Point de montage
	union {
		struct rcu_head mnt_rcu;		// Tête RC
		struct llist_node mnt_llist;		// Noeud de liste liée
	};
};


			-------- PATH --------
struct path {
	struct vfsmount *mnt;				// Pointeur vers le point de montage
	struct dentry *dentry;				// Pointeur vers la dentry
} __randomize_layout;				// Disposition aléatoire des membres
*/
