#define BEGIN_SCOPED_ENUM(Type) \
    struct Type { enum {

#define END_SCOPED_ENUM() \
    }; };

namespace drjuke
{
	BEGIN_SCOPED_ENUM(Constants)
		kMaxTaskQueueSize = 1000
	END_SCOPED_ENUM()
}