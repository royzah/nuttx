.. _capabilities:

====================
Process capabilities
====================

A process holds three capabilities. Without one, the calls it guards fail
with ``EPERM``. Every build, the kernel and init start with all three.

=================  ==================================================
``PR_CAP_RAWIO``   ``open()`` of block, MTD and BCH nodes,
                   ``mount()``, ``umount2()``
``PR_CAP_SPAWN``   ``posix_spawn()``, ``task_spawn()``,
                   ``task_create()``, ``exec()``, ``execve()``
``PR_CAP_ADMIN``   ``boardctl()`` reset and poweroff; signalling,
                   rescheduling, cancelling or renaming a thread of
                   another process (signal 0 stays open)
=================  ==================================================

.. code-block:: c

  prctl(PR_CAPS_DROP, PR_CAP_RAWIO | PR_CAP_SPAWN);
  int caps = prctl(PR_CAPS_GET);

A drop is permanent. The set lives in the task group and a new group copies
its creator's. Kernel code uses the unchecked ``file_open()``,
``nx_mount()``, ``nx_umount2()`` and the other ``nx`` variants. Raw storage
drivers register with ``register_rawdriver()``.
